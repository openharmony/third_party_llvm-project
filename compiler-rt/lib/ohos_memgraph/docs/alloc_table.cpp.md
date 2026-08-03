# alloc_table.cpp 逐函数代码解读

## 文件职责

`alloc_table.cpp` 是 formal 版 memgraph 里最核心的一层索引实现。它同时承担三件事：

1. 维护 `base -> alloc` 的精确哈希索引。
2. 维护 `addr/page -> owner alloc` 的按页范围索引。
3. 维护 alloc 节点和 range-link 节点的 slab / free-list 存储层。

因此这份文件虽然名字叫 `alloc_table`，但内部其实同时包含：

- **精确查找层**
- **范围查找层**
- **存储池层**

阅读时建议按这个顺序看，而不是按源码从上到下硬读。

---

## 相关结构体先读什么

这些结构定义在 `alloc_table.h`，但理解 `alloc_table.cpp` 之前最好先把它们过一遍。

### `AllocEntry`（`alloc_table.h:45`）

这是对外暴露的 alloc 记录快照，字段都比较“语义化”：

- `base`：对象起始地址。
- `size`：对象大小。
- `type_id / var_id`：块级 metadata 在名字表里的压缩 id。
- `store_head`：这个 alloc 自己那条 store 链的头结点。
- `id`：内部稳定 id。

可以把它理解成“去掉内部索引链和锁之后的用户视图”。

### `LockedAlloc`（`alloc_table.h:63`）

这是“已经拿住 alloc 本地 `store_mu` 之后”的交接结构。它的作用不是再存新数据，而是把下面三件事同时交给调用方：

- 这个 alloc 还是 live 的。
- 这个 alloc 的当前字段值快照已经拷出来了。
- 这个 alloc 下面的 store 链在 `Unlock()` 前不会被别人并发改掉。

### `AllocTable::Node`（`alloc_table.h:190`）

这是 alloc 表真正的内部节点。

它比 `AllocEntry` 多了几样内部实现字段：

- `hash_next_or_free`
  - live 时：是精确哈希桶链的 next。
  - free 时：复用成 free-list 的 next。
- `state`
  - `free / live / deleting`
- `store_mu`
  - 每个 alloc 独有的一把小锁，保护这个 alloc 的 store 链。

### `AllocTable::RangeLink`（`alloc_table.h:209`）

这是 page 范围索引的链节点，不是 alloc 节点本身。

它表达的是：

- 某个 `page_id`
- 对应到某个 `alloc_id`
- 并且可以和同一页里的其他候选 alloc 串成链

所以它本质上是“page bucket 里的候选表节点”。

---

## 代码分层地图

这份文件从实现职责上可以分成 8 段：

1. 基础工具函数
2. base 精确哈希辅助
3. alloc node slab / free-list
4. range-link slab / free-list
5. 精确查找和 entry 填充
6. page 范围索引维护
7. 表初始化 / 析构
8. 对外公开的插入、查找、加锁、删除接口

下面按函数顺序逐个解释。

---

## 1. 基础工具函数

### `NextPow2`（`alloc_table.cpp:37`）

作用：把输入值向上调整到最近的 2 的幂。

逐段理解：

- `37-39`：如果输入已经很小，直接返回 `1`。
- `40-44`：不断左移翻倍，直到 `pow2 >= value`。
- 返回值用于 bucket 数计算，而不是节点数计算。

为什么需要它：

- `hash_bucket_count_`
- `page_bucket_count_`

都希望是 2 的幂，这样选桶时就能用：

```cpp
hash & (bucket_count - 1)
```

而不是更贵的 `%`。

### `AllocTable::AllocTable`（`48`）

作用：把整张表的内部指针、计数器、free-list 头初始化成空状态。

重点：

- 所有 slab 指针先置空。
- `free_head_` / `range_free_head_` 置成 `-1`。
- `capacity_` / `live_count_` 等都从 `0` 开始。

这一步不分配实际内存；真正的初始化在 `Init()`。

### `Hash`（`66`）

作用：对地址值做 bit-mix，供 base 哈希和 page 哈希共用。

逐段理解：

- `66-72`：对 `uptr` 做一系列移位和乘常数，把高低位打散。
- 目的不是加密，而是让相近地址不要总落在相近桶里。

### `BaseBucket`（`76`）

作用：把 `base` 映射到精确哈希桶号。

流程：

1. 对 `base` 调 `Hash(base)`。
2. 用 `& (hash_bucket_count_ - 1)` 取模。
3. 得到 `base -> bucket`。

### `PageBucket`（`81`）

作用：把 `page_id` 映射到 page 范围索引桶号。

和 `BaseBucket()` 一样，只是 key 从 `base` 换成了 `page_id`。

### `PageIdForAddr`（`86`）

作用：把任意地址转成页号。

当前页大小固定是：

- `4KB`
- 也就是 `addr >> 12`

### `LastPageIdForRange`（`93`）

作用：计算 `[base, base + size)` 最后覆盖到哪一页。

逐段理解：

- `94-95`：`size == 0` 时单独处理，直接认为它至少覆盖 `base` 所在页。
- `96`：`size > 0` 时，用 `base + size - 1` 取最后一个真实字节，再算页号。

为什么不是 `base + size`：

- 因为区间右边是开区间。
- 真正最后属于对象的字节是 `base + size - 1`。

---

## 2. alloc node 的 slab / free-list 存储层

### `EnsureNodeSlabPtrCapacity`（`105`）

作用：确保 `node_slabs_` 这张“slab 指针数组”本身足够大。

要点：

- 这里扩的是“指针数组”，不是具体节点 slab。
- `need_count` 表示顶层数组至少能存多少个 slab 指针。
- 如果现有 `slab_ptr_cap_` 足够，就直接返回。
- 不够就申请更大的指针数组，并把旧内容拷过去。

理解方式：

- `node_slabs_[i]` 是“第 i 块 slab 的入口”。
- 这个函数负责保证“入口数组能放得下更多 slab”。

### `EnsureNodeSlabForIndex`（`136`）

作用：确保“包含这个节点索引的那块 slab”已经真实分配出来。

逐段理解：

- 先算 `slab_idx = idx / kNodesPerSlab`。
- 如果上层数组都还不够大，就先调用 `EnsureNodeSlabPtrCapacity`。
- 如果 `node_slabs_[slab_idx]` 还没分配，就申请一整块 `Node[kNodesPerSlab]`。
- 把这块新 slab 里的节点状态初始化为 free。

和上一个函数的区别：

- `EnsureNodeSlabPtrCapacity` 管“目录”
- `EnsureNodeSlabForIndex` 管“具体那一栋楼是否盖出来”

### `GrowNodeSlabs`（`166`）

作用：按 slab 粒度为 alloc node 扩容。

逐段理解：

- 先看当前 `capacity_` 对应的下一个节点索引。
- 调 `EnsureNodeSlabForIndex(capacity_)`，保证这块 slab 存在。
- 然后把可用 `capacity_` 往上增加一整个 slab 的大小。

它不会一次加一个节点，而是一次加一整块 slab。

### `GetNode`（`180` / `194 const`）

作用：把稳定 `id` 映射回真实 `Node *`。

流程很固定：

1. `slab_idx = id / kNodesPerSlab`
2. `slot_idx = id % kNodesPerSlab`
3. 返回 `&node_slabs_[slab_idx][slot_idx]`

这是当前 alloc 节点“id 化”的核心。

### `AcquireNode`（`216`）

作用：拿一个可用的 alloc 节点 id。

优先级：

1. 先看 free-list。
2. free-list 没有，再看 `next_node_` 是否还能从新容量里拿。
3. 容量不够就 `GrowNodeSlabs()`。

逐段理解：

- `219-228`：优先弹出 free-list 头。
- `229-239`：如果需要，从尾部未使用区域拿一个新 id。
- `240-241`：失败时返回 `-1`。

### `ReleaseNode`（`243`）

作用：把一个 alloc 节点回收到 free-list。

做的事很简单：

- 清空关键字段
- 把 `state` 改回 `kNodeFree`
- 用 `hash_next_or_free` 复用成 free-list next
- 头插回 `free_head_`

---

## 3. range-link 的 slab / free-list 存储层

### `EnsureRangeSlabPtrCapacity`（`258`）

和 `EnsureNodeSlabPtrCapacity()` 同构，只不过对象从 `Node` 换成了 `RangeLink`。

负责：

- 扩 `range_slabs_` 这张顶层指针数组

### `EnsureRangeSlabForIndex`（`286`）

和 `EnsureNodeSlabForIndex()` 同构。

负责：

- 计算某个 `RangeLink` 索引所在的 slab
- 确保那块真实 `RangeLink[kRangeLinksPerSlab]` 已分配

### `GrowRangeLinkSlabs`（`315`）

作用：按 slab 粒度为范围索引节点扩容。

### `GetRangeLink`（`323` / `337 const`）

作用：把稳定 `range link id` 映射回真实 `RangeLink *`。

这和 `GetNode()` 完全同构。

### `AcquireRangeLink`（`356`）

作用：获取一个可用的 `RangeLink` 槽位。

逻辑同样是：

1. 先 free-list
2. 再新 slot
3. 不够则 grow

### `ReleaseRangeLink`（`377`）

作用：把一个范围索引节点回收进 free-list。

---

## 4. 精确查找和锁住 alloc 的公共积木

### `FindIdInHashBucketLocked`（`391`）

作用：在“已经拿住桶锁”的前提下，在某个 bucket 链里找 `base` 对应的节点 id。

逐段理解：

- 从 `hash_buckets_[bucket]` 开始。
- 每次 `GetNode(id)` 拿到节点。
- 比较：
  - `state == kNodeLive`
  - `node->base == base`
- 命中就返回 id。
- 否则顺着 `hash_next_or_free` 往后走。

这是精确哈希查找的底层核心。

### `FillEntryLocked`（`410`）

作用：把内部 `Node` 的语义字段拷到 `AllocEntry`。

它不做查找，只做“内部节点 -> 外部快照”的字段映射。

### `LockNodeIfLive`（`430`）

作用：如果某个节点还 live，就拿住它的 `store_mu`，并把内容填入 `LockedAlloc`。

逐段理解：

- 先按 `id` 找到 `Node`。
- 检查 `state`，不是 live 直接失败。
- 拿 `node->store_mu`。
- 再次确认节点没在拿锁间隙被改状态。
- 把字段拷给 `LockedAlloc`。

它解决的是：

- “找到了 alloc”
- 和
- “这个 alloc 的 store 链接下来可安全访问”

之间的衔接。

---

## 5. page 范围索引维护

### `InsertRangeLinksForNode`（`456`）

作用：把一个 alloc 覆盖到的每个页，都登记到 page 范围索引里。

流程：

1. 先算第一页和最后一页。
2. 对每一页：
   - 申请一个 `RangeLink`
   - 填 `page_id`
   - 填 `alloc_id`
   - 头插进对应的 page bucket 链

这一步完成后，`FindContaining()` 才能通过 page 候选集找到这个 alloc。

### `RollbackInsertedRangeLinks`（`484`）

作用：如果 `InsertRangeLinksForNode()` 中途失败，回滚已经插进去的那部分页链。

这是 `Insert()` 的失败恢复路径，避免：

- 精确哈希已插入
- 范围索引只插了一半

导致结构不一致。

### `RemoveRangeLinksForNode`（`523`）

作用：删除某个 alloc 覆盖到的所有页链节点。

流程：

1. 重新算出 `[first_page, last_page]`
2. 在每一页对应的 bucket 链里找出 `alloc_id` 对应的那些 `RangeLink`
3. 从链里摘掉
4. 回收到 range-link free-list

这一步通常在 `free(base)` 的删除流程里执行。

---

## 6. 表级生命周期

### `Init`（`560`）

作用：初始化整张 `AllocTable`。

逐段理解：

- 记录最大容量。
- 根据 `max_capacity_ * 2` 估出精确哈希桶数和 page 桶数，再用 `NextPow2` 调成 2 的幂。
- 分配：
  - `hash_buckets_`
  - `page_buckets_`
  - 对应的锁数组
- 把所有桶头初始化成 `-1`

它只准备桶和元数据，不会一次性分满所有 alloc 节点 slab。

### `Destroy`（`599`）

作用：释放整张表持有的所有资源。

会清理：

- alloc node slabs
- range-link slabs
- bucket 数组
- 锁数组

并把计数器、指针和 free-list 头重置。

---

## 7. 对外核心接口：插入与查找

### `Insert`（`673`）

作用：插入一个新的 alloc。

这是整张表最重要的写路径之一。

完整流程：

1. 检查 live alloc 容量是否已到上限。
2. 用 `AcquireNode()` 拿一个新节点。
3. 填好节点的：
   - `base`
   - `size`
   - `type_id / var_id`
   - `store_head`
   - `state`
4. 算出 `base` 对应的精确哈希 bucket。
5. 在桶锁保护下：
   - 检查是否已有同 `base`
   - 没有则把新节点头插进哈希桶链
6. 再调用 `InsertRangeLinksForNode()` 把页索引补齐。
7. 如果范围索引失败：
   - 从哈希桶回滚
   - 释放 alloc 节点

这一版特别重要的设计点是：

- **先发布精确索引**
- **再发布范围索引**
- **失败时明确回滚**

### `Find`（`751`）

作用：按 `base` 精确查 alloc，并填 `AllocEntry`。

流程很轻：

1. 算 bucket
2. 拿桶锁
3. 调 `FindIdInHashBucketLocked`
4. 命中后 `FillEntryLocked`

### `FindContaining`（`772`）

作用：按任意地址查“哪个 alloc 包含这个地址”。

这是 `store_record(dst_ptr)` 的关键依赖。

流程：

1. 算出地址所在页 `page_id`
2. 算出对应的 page bucket
3. 遍历这个 bucket 链里的 `RangeLink`
4. 通过 `alloc_id -> Node`
5. 再做真正的范围判断：
   - `node->base <= addr < node->base + node->size`
6. 命中则返回 `AllocEntry`

注意：

- page bucket 只是候选集
- 真正归属还是靠最后那次范围判断确定

### `FindId`（`792`）

作用：只按 `base` 找稳定 `id`，不构造完整 `AllocEntry`。

适合“只要 id，不要整条快照”的场景。

### `GetById`（`808`）

作用：已知稳定 id，直接回查 alloc 记录。

它绕过了精确哈希，不用再按 `base` 查。

---

## 8. 对外核心接口：带锁查询与原地修改

### `LockByBase`（`824`）

作用：按 `base` 找 alloc，并把这个 alloc 的 `store_mu` 拿住。

典型调用方：

- `get_member_info`
- `alloc_record`

### `LockContaining`（`843`）

作用：按任意地址找 owner alloc，并拿住它的 `store_mu`。

典型调用方：

- `store_record`

这里比 `FindContaining()` 多做了一步“锁住 owner”。

### `Unlock`（`879`）

作用：释放 `LockedAlloc` 持有的 `store_mu`。

调用后：

- `locked->locked = false`
- 该 alloc 不再处于“稳定快照”状态

### `UpdateLockedMeta`（`893`）

作用：在已经拿住 `store_mu` 的前提下，更新块级 metadata。

通常就是更新：

- `type_id`
- `var_id`

### `UpdateLockedSize`（`907`）

作用：在已锁 alloc 上更新 `size`。

主要服务于：

- `realloc(old_base == new_base)` 这种同地址 resize

### `SetLockedStoreHead`（`919`）

作用：在已锁 alloc 上更新 `store_head`。

它把“alloc 自己挂着哪条 store 链”的头指针改掉。

---

## 9. 删除流程三段式

### `BeginRemove`（`940`）

作用：删除流程第一步。

做的事：

1. 按 `base` 在精确哈希里找到 alloc。
2. 从哈希桶链里摘掉它。
3. 把节点状态改成 `kNodeDeleting`。
4. 把 alloc 内容拷到 `removed` 给上层继续用。

这一阶段的关键是：

- alloc 从“精确查找可见”变成“逻辑上已经下线”
- 但物理节点还没回收

### `RemoveRangeForEntry`（`982`）

作用：删除流程第二步。

用刚才保存下来的 `AllocEntry`：

- 重新算它覆盖的页范围
- 把这些页里的 `RangeLink` 全删掉

也就是让它从“按地址反查 owner”的路径里也下线。

### `FinalizeRemove`（`993`）

作用：删除流程第三步。

真正做物理回收：

- 清空节点内容
- 改状态为 free
- 放回 alloc free-list

### `Remove`（`1018`）

作用：三段式删除的便捷封装。

内部顺序就是：

1. `BeginRemove`
2. `RemoveRangeForEntry`
3. `FinalizeRemove`

这正是 `free(base)` 路径的 alloc 层删除模型。

---

## 10. 统计辅助函数

### `Size`（`1035`）

返回当前 live alloc 数。

### `StorageEntrySize`（`1040`）

返回一个内部 `Node` 的物理大小。

它不是逻辑记录数接口，而是给：

- `get_layout`
- 观测 / benchmark

提供行大小估算。

---

## 最后怎么把这份文件讲给别人听

如果你要现场讲这份文件，建议用这条主线：

1. **先讲三层职责**
   - 精确哈希
   - page 范围索引
   - slab 存储池
2. **再讲两个核心写路径**
   - `Insert`
   - `Remove`
3. **再讲两个核心查路径**
   - `Find`
   - `FindContaining`
4. **最后补并发点**
   - 精确哈希按 bucket 锁
   - 范围索引按 page bucket 锁
   - 每个 alloc 自己再有 `store_mu`

这样听众最容易建立整体图景，不会一上来就陷进 slab 细节里。
