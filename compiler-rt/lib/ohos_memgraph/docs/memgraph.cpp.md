# memgraph.cpp 逐函数代码解读

## 文件职责

`memgraph.cpp` 是 formal 版 runtime 的“总调度层”。

它不负责实现每张表的底层细节，而是把下面这些能力串起来：

- 初始化全局 runtime
- 接住 malloc/free/realloc hook
- 接住 `alloc_record` / `store_record`
- 提供 `get_block_info` / `get_member_info`
- 管理 flags、懒初始化和 observability 开关

如果说：

- `alloc_table.cpp` 是 alloc 索引层
- `store_table.cpp` 是 store 节点池
- `name_table.cpp` 是名字压缩层

那么 `memgraph.cpp` 就是“把这些部件编排成真正运行时行为”的文件。

---

## 先读哪些全局状态

这些声明在 `memgraph.h`。

### 全局表对象（`memgraph.h:42-45`）

- `alloc_table`
- `store_table`
- `type_table`
- `var_table`

这是 runtime 真正的核心数据对象。

### 初始化与拦截控制状态（`32-35`）

- `ohos_memgraph_inited`
- `ohos_memgraph_init_is_running`
- `ohos_memgraph_disable_interceptors`

它们解决：

- 是否已经初始化
- 初始化过程中避免递归
- 某些内部路径临时禁止 hook 再次打进来

### 顶层锁（`52-54`）

- `alloc_mu`
- `store_mu`
- `graph_mu`

formal 版的主要并发控制已经下沉到更细粒度的桶锁和 `store_mu`，但这些顶层锁仍保留给 runtime 流程编排使用。

---

## 代码主线

这份文件可以按 6 段来理解：

1. 内部 helper
2. flag / 环境变量处理
3. 初始化
4. malloc/free/realloc hook 路径
5. 前端 metadata 写路径
6. IDE 主查询路径

---

## 1. 内部 helper

### `AllocateInternalObject<T>`（`45`）

作用：给 runtime 自己分配内部对象。

它通常用于：

- 分配 `AllocTable`
- 分配 `StoreTable`
- 分配 `NameTable`

这一步会走 sanitizer 的内部分配器，而不是普通用户 `malloc`。

### `ScopedDisableInterceptors`（`52`）

作用：一个小的 RAII helper。

逻辑是：

- 构造时：把 `ohos_memgraph_disable_interceptors` 加一
- 析构时：减一

这样 runtime 在执行自己内部需要用到 libc/allocator 的代码时，可以暂时让 hook 旁路，避免无限递归。

### `ClampSize`（`60`）

作用：把 `uptr size` 压到当前 runtime 能接受的 `u32` 范围里。

因为 alloc/store 表内部把 `size` 存成 `u32`，所以这里要做上界裁剪。

### `ClampPositiveEnvToInt`（`66`）

作用：从环境变量读取一个正整数配置。

逻辑：

- 环境变量缺失：返回 fallback
- 读到非正数或非法值：返回 fallback
- 合法：返回解析值

它用于诸如 alloc/store 容量配置。

### `ClampBoolEnv`（`76`）

作用：从环境变量读取布尔开关。

典型用法是：

- `OHOS_MEMGRAPH_ENABLED`
- `OHOS_MEMGRAPH_OBSERVABILITY_ENABLED`

### `HasFrontendMeta`（`90`）

作用：判断前端传进来的字符串指针是否可视为“有 metadata”。

逻辑很简单：

- 非空且首字符非 `'\0'`

### `RefreshLiveCountersLocked`（`94`）

作用：把当前 authoritative 的：

- live alloc 数
- live store 数

发布给 observability 统计层。

它假定调用方已经在合适的结构锁保护下。

---

## 2. flag / 环境变量

### `Flags::SetDefaults`（`106`）

作用：设置 runtime 默认配置。

它会决定：

- runtime 是否启用
- observability 是否启用
- alloc / store 最大容量

这一步是初始化时的默认值基线，之后再叠加环境变量覆盖。

### `flags`（`121`）

作用：返回全局 `Flags` 对象。

### `HooksEnabled`（`122`）

作用：告诉上层：

- malloc/free/realloc hook 是否应该真正参与追踪

### `ObservabilityEnabled`（`123`）

作用：告诉上层：

- `get_info / get_runtime_stats / get_layout`
- atexit summary

这些维测逻辑是否启用

这也是 `OHOS_MEMGRAPH_OBSERVABILITY_ENABLED=0` 这个开关真正生效的位置之一。

---

## 3. 初始化

### `Initialize`（`131`）

作用：懒初始化整个 runtime。

这是 `memgraph.cpp` 里最重要的总入口之一。

完整流程：

1. 处理重复初始化：
   - 已初始化直接返回
   - 初始化正在进行时直接返回
2. 设置 `ohos_memgraph_init_is_running`
3. 用 `ScopedDisableInterceptors` 防止初始化过程再被 malloc hook 打进来
4. 创建：
   - `alloc_table`
   - `store_table`
   - `type_table`
   - `var_table`
5. 读取 flags 和环境变量
6. 初始化四张核心表
7. 安装拦截器
8. 设置：
   - `ohos_memgraph_inited = 1`
   - `ohos_memgraph_init_is_running = false`

理解这段代码时要抓住一点：

- runtime 是懒初始化的
- 不要求用户显式先调用 `memgraph_init()`

---

## 4. malloc/free/realloc hook 路径

### `TrackHookAlloc`（`170`）

作用：处理一次新对象分配。

完整流程：

1. 如果 runtime 未启用或地址为空，直接返回
2. 保证 runtime 已初始化
3. 记录一次 malloc hook 事件
4. 裁剪 size
5. 往 `alloc_table` 插入一条新的 alloc
6. 刷新 live 计数

这里不写块级 metadata，只做“对象生命周期入表”。

### `TrackHookFree`（`199`）

作用：处理一次对象释放。

完整流程：

1. 如果 runtime 未启用或地址为空，直接返回
2. 记录一次 free hook 事件
3. 从 `alloc_table` 开始三段式删除 alloc
4. 如果这个 alloc 下面还有 store 链：
   - 让 `store_table` 清理整条链
5. 刷新 live 计数

这是 alloc 和 store 两层真正汇合的地方之一。

### `TrackHookRealloc`（`218`）

作用：处理 realloc 语义。

它要分情况：

1. `old_base == 0`
   - 退化成 alloc
2. `new_base == 0`
   - 退化成 free
3. `old_base == new_base`
   - 同地址 resize，只更新 size
4. `old_base != new_base`
   - 先 free 旧对象，再 alloc 新对象

这也是为什么 `AllocTable` 有 `UpdateLockedSize()` 这条路径。

---

## 5. 前端 metadata 写路径

### `RecordMallocMetadata`（`250`）

作用：给某个 tracked alloc 写块级 metadata。

完整流程：

1. 检查：
   - runtime 是否启用
   - `base` 是否有效
   - type/name 是否真的有前端 metadata
2. 记录一次 `alloc_record` 事件
3. 用 `type_table` / `var_table` 把字符串 intern 成 id
4. 通过 `alloc_table.LockByBase(base)` 找到并锁住这个 alloc
5. 调 `UpdateLockedMeta()` 更新 alloc 节点里的：
   - `type_id`
   - `var_id`
6. 解锁

这里不创建 store 节点，因为这是块级 metadata。

### `RecordStoreMetadata`（`287`）

作用：给某个字段地址写成员级 metadata。

这是当前 formal 设计里最有代表性的一条写路径。

完整流程：

1. 检查 runtime 和 metadata 有效性
2. 记录一次 `store_record` 事件
3. 先把 type/name intern 成 id
4. 用 `alloc_table.LockContaining(dst_ptr)` 找到 owner alloc，并锁住它
5. 计算：
   - `offset = dst_ptr - owner.base`
6. 调 `store_table.UpsertRecord(...)` 新建一个 store 历史节点
7. 用 `alloc_table.SetLockedStoreHead(...)` 把 owner 的链头改成新节点
8. 解锁 owner
9. 刷新 live 计数

这条路径最能体现当前 formal 方案：

- owner 查找靠 page 范围索引
- store 存储靠 per-alloc 链

---

## 6. IDE 主查询路径

### `GetBlockInfo`（`337`）

作用：按 `base` 查询对象本身的块级 metadata。

流程：

1. 检查参数
2. 用 `alloc_table.Find(base)` 精确找到 alloc
3. 用 `type_table.Resolve(type_id)` / `var_table.Resolve(var_id)` 反解字符串
4. 填 `block_info_t`

这条路径只走精确哈希，不走 page 范围索引。

### `GetMemberInfo`（`358`）

作用：按 `base + member_addr` 查询字段级 metadata。

流程：

1. 检查参数
2. 用 `LockByBase(base)` 找到 owner alloc 并锁住它
3. 算出 `offset = member_addr - base`
4. 在 owner 的 store 链里调用 `store_table.Find(store_head, offset, ...)`
5. 把 store 记录里的 `type_id / var_id` 反解回字符串
6. 填 `member_info_t`
7. 解锁 owner

注意这里不需要 `FindContaining()`，因为调用方已经给了 `base`。

---

## 怎么把这份文件讲给别人

建议你现场讲时只抓这 5 条主线：

1. `Initialize`
2. `TrackHookAlloc / TrackHookFree / TrackHookRealloc`
3. `RecordMallocMetadata`
4. `RecordStoreMetadata`
5. `GetBlockInfo / GetMemberInfo`

这几条已经把 runtime 的主要行为全串起来了。

一句话概括：

> `memgraph.cpp` 不自己维护复杂数据结构，它负责把 alloc/store/name 三张核心表和 hook / query / flags 编排成真正的运行时行为。
