# OHOS Memgraph Runtime 中文代码检视文档

本文面向代码评审、走读和对外讲解，覆盖当前正式版本 `compiler-rt/lib/ohos_memgraph` 的主要源码文件。目标是帮助读者快速建立以下认知：

- 这个运行时库整体在解决什么问题
- 每个文件负责哪一层能力
- 每个结构体和函数分别承担什么职责
- 主要功能路径如何从 `malloc/free` 和前端写入接口流转到查询接口

本文默认以“主库源码”为重点，测试目录单独放在附录章节说明其验证目标和主流程。

---

## 1. 总体定位

`ohos_memgraph` 是一个放在 `compiler-rt` 下的低层运行时库，用来为堆对象维护两类 metadata：

- **块级 metadata**
  - 例如一个堆对象本身的 `type_name / var_name`
  - 通过 `alloc_record()` 写入
- **成员级 metadata**
  - 例如对象内部某个字段地址对应的 `type_name / var_name`
  - 通过 `store_record()` 写入

它同时还提供：

- 面向 IDE / 查询侧的主接口：
  - `get_block_info()`
  - `get_member_info()`
- 面向诊断 / benchmark / 观测的辅助接口：
  - `get_info()`
  - `get_info_records()`
  - `get_runtime_stats()`
  - `get_layout()`

当前正式版本的内部设计有三个关键点：

1. **AllocTable**
   - 用“精确哈希 + 按页范围索引”管理 live alloc
2. **StoreTable**
   - 用 slab 节点池管理“每个 alloc 自己的 store 链”
3. **NameTable**
   - 把前端传入的字符串指针压缩成紧凑的整数 id

---

## 2. 目录结构总览

主库源码目录：

- [CMakeLists.txt](/srv/workspace/memgraph/memgraph_formal_master/compiler-rt/lib/ohos_memgraph/CMakeLists.txt)
- [memgraph_interface.h](/srv/workspace/memgraph/memgraph_formal_master/compiler-rt/lib/ohos_memgraph/memgraph_interface.h)
- [memgraph_flags.h](/srv/workspace/memgraph/memgraph_formal_master/compiler-rt/lib/ohos_memgraph/memgraph_flags.h)
- [memgraph_flags.inc](/srv/workspace/memgraph/memgraph_formal_master/compiler-rt/lib/ohos_memgraph/memgraph_flags.inc)
- [memgraph_stats_internal.h](/srv/workspace/memgraph/memgraph_formal_master/compiler-rt/lib/ohos_memgraph/memgraph_stats_internal.h)
- [name_table.h](/srv/workspace/memgraph/memgraph_formal_master/compiler-rt/lib/ohos_memgraph/name_table.h)
- [name_table.cpp](/srv/workspace/memgraph/memgraph_formal_master/compiler-rt/lib/ohos_memgraph/name_table.cpp)
- [alloc_table.h](/srv/workspace/memgraph/memgraph_formal_master/compiler-rt/lib/ohos_memgraph/alloc_table.h)
- [alloc_table.cpp](/srv/workspace/memgraph/memgraph_formal_master/compiler-rt/lib/ohos_memgraph/alloc_table.cpp)
- [store_table.h](/srv/workspace/memgraph/memgraph_formal_master/compiler-rt/lib/ohos_memgraph/store_table.h)
- [store_table.cpp](/srv/workspace/memgraph/memgraph_formal_master/compiler-rt/lib/ohos_memgraph/store_table.cpp)
- [memgraph.h](/srv/workspace/memgraph/memgraph_formal_master/compiler-rt/lib/ohos_memgraph/memgraph.h)
- [memgraph.cpp](/srv/workspace/memgraph/memgraph_formal_master/compiler-rt/lib/ohos_memgraph/memgraph.cpp)
- [memgraph_interceptors.cpp](/srv/workspace/memgraph/memgraph_formal_master/compiler-rt/lib/ohos_memgraph/memgraph_interceptors.cpp)
- [memgraph_allocation_functions.cpp](/srv/workspace/memgraph/memgraph_formal_master/compiler-rt/lib/ohos_memgraph/memgraph_allocation_functions.cpp)
- [memgraph_monitoring.cpp](/srv/workspace/memgraph/memgraph_formal_master/compiler-rt/lib/ohos_memgraph/memgraph_monitoring.cpp)
- [ohos_memgraph_minimal_import.patch](/srv/workspace/memgraph/memgraph_formal_master/compiler-rt/lib/ohos_memgraph/ohos_memgraph_minimal_import.patch)

测试目录：

- [tests/](/srv/workspace/memgraph/memgraph_formal_master/compiler-rt/lib/ohos_memgraph/tests)

---

## 3. 主流程图

### 3.1 写路径

1. 应用执行 `malloc/calloc/realloc/free`
2. interceptor 进入：
   - [memgraph_interceptors.cpp](/srv/workspace/memgraph/memgraph_formal_master/compiler-rt/lib/ohos_memgraph/memgraph_interceptors.cpp)
3. 转入：
   - `TrackHookAlloc()`
   - `TrackHookFree()`
   - `TrackHookRealloc()`
4. `alloc_record()` / `store_record()` 再分别写块级 / 成员级 metadata

### 3.2 查询路径

1. IDE 或测试调用：
   - `get_block_info(base)`
   - `get_member_info(base, member_addr)`
2. C ABI wrapper 转到内部：
   - `GetBlockInfo()`
   - `GetMemberInfo()`
3. `GetBlockInfo()` 走 alloc 精确哈希
4. `GetMemberInfo()` 先用 `base` 找 alloc，再在 owner-local store 链里找 offset

---

## 4. 文件级走读

## 4.1 `CMakeLists.txt`

文件：
- [CMakeLists.txt](/srv/workspace/memgraph/memgraph_formal_master/compiler-rt/lib/ohos_memgraph/CMakeLists.txt)

### 总体作用

定义 `ohos_memgraph` 这个 compiler-rt 组件，并分别生成：

- `libclang_rt.memgraph.a`
- `libclang_rt.memgraph.so`

### 关键逻辑

- `add_compiler_rt_component(memgraph)`
  - 声明一个名为 `memgraph` 的 compiler-rt 组件

- `OHOS_MEMGRAPH_SOURCES`
  - 列出正式运行时的全部 `.cpp` 源文件

- `OHOS_MEMGRAPH_HEADERS`
  - 列出参与构建和导出的头文件

- `OHOS_MEMGRAPH_CFLAGS`
  - 为运行时设置统一编译选项

- `add_memgraph_runtime(runtime_name, arch)`
  - 为指定架构生成静态库和动态库

### 检视重点

- 正式 runtime 只接入 `ohos_memgraph` 目录，不再依赖 `v2/indexed` 目录
- 构建名已经收敛为 `clang_rt.memgraph`

---

## 4.2 `memgraph_interface.h`

文件：
- [memgraph_interface.h](/srv/workspace/memgraph/memgraph_formal_master/compiler-rt/lib/ohos_memgraph/memgraph_interface.h)

### 总体作用

对外公开稳定的 C ABI，是前端插桩、IDE 查询、测试和 benchmark 都会使用的接口层。

### 结构体说明

- `struct block_info`
  - `get_block_info()` 的输出结构
  - 描述一个堆对象本身的块级 metadata

- `struct member_info`
  - `get_member_info()` 的输出结构
  - 描述对象内部某个字段的成员级 metadata

- `struct alloc_info`
  - `get_info()` 的输出结构
  - 提供 allocation 是否存在、大小、挂了多少条 metadata 记录

- `struct info_record`
  - `get_info_records()` 的输出结构
  - 枚举一个 alloc 相关的 metadata 记录

- `struct runtime_stats`
  - `get_runtime_stats()` 的输出结构
  - 汇总 runtime 内部内存占用、峰值、事件计数、表容量等信息

### 函数说明

- `alloc_record(unsigned long malloc_addr, const char *type_name, const char *var_name)`
  - 给一个堆对象写块级 metadata

- `store_record(unsigned long source_addr, unsigned long dst_ptr, const char *type_name, const char *var_name)`
  - 给一个字段地址写成员级 metadata
  - 当前实现里 `source_addr` 只保留 ABI，不参与核心逻辑

- `get_block_info(unsigned long base, block_info_t *out)`
  - 查询对象本身的 metadata

- `get_member_info(unsigned long base, unsigned long member_addr, member_info_t *out)`
  - 查询对象内某个字段地址的 metadata

- `get_info(unsigned long base, alloc_info_t *out)`
  - 诊断接口，返回对象摘要信息

- `get_info_records(unsigned long base, info_record_t *out, unsigned long capacity)`
  - 枚举对象相关的所有 metadata 记录

- `get_runtime_stats(runtime_stats_t *out)`
  - 获取 runtime 观测统计

- `get_layout(unsigned long *alloc_row_bytes, unsigned long *store_row_bytes)`
  - 获取 alloc row / store row 的物理大小

- `memgraph_init(void)`
  - 手动初始化入口

---

## 4.3 `memgraph_flags.h` / `memgraph_flags.inc`

文件：
- [memgraph_flags.h](/srv/workspace/memgraph/memgraph_formal_master/compiler-rt/lib/ohos_memgraph/memgraph_flags.h)
- [memgraph_flags.inc](/srv/workspace/memgraph/memgraph_formal_master/compiler-rt/lib/ohos_memgraph/memgraph_flags.inc)

### 总体作用

定义 runtime 级别的配置项，支持：

- 启停 runtime
- 设置 alloc/store 容量上限
- 关闭 observability

### 结构体说明

- `struct Flags`
  - 聚合全部运行时配置项

### 字段说明

- `enabled`
  - runtime 总开关

- `observability_enabled`
  - 维测 / 诊断接口是否生效

- `alloc_table_size`
  - live alloc 上限

- `store_table_size`
  - live store row 上限

- `verbose`
  - 预留开关，当前未使用

### 函数说明

- `Flags::SetDefaults()`
  - 从默认值和环境变量组合出最终运行时配置

- `flags()`
  - 返回全局 Flags 实例

---

## 4.4 `memgraph_stats_internal.h`

文件：
- [memgraph_stats_internal.h](/srv/workspace/memgraph/memgraph_formal_master/compiler-rt/lib/ohos_memgraph/memgraph_stats_internal.h)

### 总体作用

定义内部观测层使用的 hook 声明。主功能路径只“写入这些统计”，不依赖其值做正确性判断。

### 函数说明

- `MemStatsOnAllocTableAlloc/Free`
  - 记录 alloc table 内存增减

- `MemStatsOnStoreTableAlloc/Free`
  - 记录 store table 内存增减

- `MemStatsOnTypeTableAlloc/Free`
  - 记录 type name table 内存增减

- `MemStatsOnVarTableAlloc/Free`
  - 记录 var name table 内存增减

- `MemStatsOnMiscAlloc/Free`
  - 记录其他 runtime-owned 对象内存增减

- `MemStatsOnMallocHookCall/FreeHookCall/ReallocHookCall`
  - 记录 hook 调用次数

- `MemStatsOnMallocRecordCall/StoreRecordCall`
  - 记录元数据写入次数

- `MemStatsUpdateLiveCounters`
  - 把 authoritative functional tables 的 live 数发布到监控层

- `MemStatsGetSnapshot`
  - 构造对外可见的 `runtime_stats_t`

- `MemStatsLogSummary`
  - 进程退出时打印 runtime 摘要

---

## 4.5 `name_table.h`

文件：
- [name_table.h](/srv/workspace/memgraph/memgraph_formal_master/compiler-rt/lib/ohos_memgraph/name_table.h)

### 总体作用

把前端传入的 `const char *` 字符串指针压缩成紧凑的 `u32 id`，并支持从 id 再解回字符串。

### 枚举说明

- `enum NameTableKind`
  - `kTypeNameTable`
    - type 名字表
  - `kVarNameTable`
    - variable 名字表

### 结构体/类说明

- `class NameTable`
  - 名字表实现主体

- `struct Slot`
  - 内部哈希表的一个槽位
  - 保存：
    - 原始字符串指针
    - 分配出的 id
    - 槽位状态

### 函数说明

- `NameTable()`
  - 构造空表

- `Init(NameTableKind kind, uptr initial_map_capacity)`
  - 初始化名字表种类和底层存储

- `Destroy()`
  - 释放底层 map 和 id 数组

- `Intern(const char *ptr)`
  - 把字符串指针 intern 成一个 `u32 id`
  - 去重策略基于**指针身份**，不是字符串内容

- `Resolve(u32 id) const`
  - 把 id 反查回原始字符串指针

- `MapEntrySize() const`
  - 返回内部哈希槽位大小

- `OnAlloc(uptr bytes)`
  - 将内存增量记入 type 或 var 统计项

- `OnFree(uptr bytes)`
  - 将内存释放记回 type 或 var 统计项

- `Hash(const char *ptr) const`
  - 对字符串指针地址做 hash

- `Rehash(uptr new_cap)`
  - 扩容并重建 open-addressing 哈希表

- `EnsureIdCapacity()`
  - 确保 id -> ptr 数组足够大

---

## 4.6 `name_table.cpp`

文件：
- [name_table.cpp](/srv/workspace/memgraph/memgraph_formal_master/compiler-rt/lib/ohos_memgraph/name_table.cpp)

### 总体作用

实现 `NameTable` 的 pointer-identity interning 逻辑。运行时通过它把反复出现的 type/var 字符串指针压缩为整数 id，减少 alloc/store row 中的存储负担。

### 关键实现点

- 内部哈希表使用 **open addressing**
- `id 0` 保留为“没有 metadata”
- 实际可用 id 从 `1` 开始
- rehash 时只搬迁活跃槽位，不改变已分配 id

### 主要函数说明

- `NameTable::NameTable()`
  - 初始化所有成员为“空表状态”

- `OnAlloc()` / `OnFree()`
  - 依据 `kind_` 将内存计入 type 或 var 类别

- `Hash(const char *ptr) const`
  - 直接对指针地址做 bit-mix

- `Rehash(uptr new_cap)`
  - 申请新 map，把旧活跃槽位重散列到新表

- `EnsureIdCapacity()`
  - 当 `id_to_ptr_` 空间不足时扩容

- `Init()`
  - 重置表并初始化 map 与 id 数组

- `Destroy()`
  - 释放 map 与 id 数组

- `Intern(const char *ptr)`
  - 返回 compact id
  - 如果该指针已存在，直接复用旧 id

- `Resolve(u32 id) const`
  - 给查询路径把 id 解成原始字符串指针

- `MapEntrySize() const`
  - 返回内部 `Slot` 大小

---

## 4.7 `alloc_table.h`

文件：
- [alloc_table.h](/srv/workspace/memgraph/memgraph_formal_master/compiler-rt/lib/ohos_memgraph/alloc_table.h)

### 总体作用

`AllocTable` 是正式版本的 alloc 主索引。它维护两层索引：

1. **精确哈希索引**
   - `base -> alloc`
2. **page 范围索引**
   - `page -> candidate alloc`

它同时还负责：

- alloc 节点的 slab 存储与复用
- alloc 稳定 id 的分配
- 为每个 alloc 提供本地 `store_mu`

### 结构体说明

- `struct AllocEntry`
  - alloc 的对外可读视图
  - 不包含内部索引链和锁

- `struct LockedAlloc`
  - 已经拿住某个 alloc 的本地 `store_mu` 的交接对象
  - 用于 `get_member_info()`、`store_record()`、`free()` 等路径

- `class AllocTable`
  - alloc 主索引类

- `enum NodeState`
  - alloc 节点状态：
    - `kNodeFree`
    - `kNodeLive`
    - `kNodeDeleting`

- `struct Node`
  - alloc table 真正存储的一条内部记录
  - 包含：
    - 块级 metadata
    - `store_head`
    - 哈希桶链 / free-list 复用字段
    - 状态位
    - `store_mu`

- `struct RangeLink`
  - page 范围索引里的一条链节点
  - 表示“某个 page 上有一个 alloc 候选”

### 常量说明

- `kNodesPerSlab`
  - 每个 alloc node slab 放多少个 `Node`

- `kRangeLinksPerSlab`
  - 每个 range-link slab 放多少个 `RangeLink`

- `kPageShift`
  - 当前页索引的页大小位移，`12` 表示 4KB 一页

### 对外函数说明

- `AllocTable()`
  - 构造空表

- `Init(uptr initial_capacity)`
  - 初始化 alloc table、bucket、锁和容量上限

- `Destroy()`
  - 销毁 alloc table 内部全部结构

- `Insert(uptr base, u32 size, s32 *out_id)`
  - 插入一个新的 alloc
  - 同时发布到精确哈希和 page 范围索引

- `Find(uptr base, AllocEntry *out) const`
  - 按 `base` 精确查找 alloc

- `FindContaining(uptr addr, AllocEntry *out) const`
  - 按地址查找“哪个 alloc 包含它”

- `FindId(uptr base, s32 *out_id) const`
  - 轻量版精确查找，只返回稳定 id

- `GetById(s32 id, AllocEntry *out) const`
  - 用稳定 id 反查 alloc 内容

- `LockByBase(uptr base, LockedAlloc *out) const`
  - 按 `base` 找 alloc 并拿住这个 alloc 的 `store_mu`

- `LockContaining(uptr addr, LockedAlloc *out) const`
  - 按地址找 owner alloc 并拿住它的 `store_mu`

- `Unlock(LockedAlloc *locked) const`
  - 释放上述 local store lock

- `UpdateLockedMeta(LockedAlloc *locked, u32 type_id, u32 var_id)`
  - 在 alloc 已锁定的前提下更新块级 metadata

- `UpdateLockedSize(LockedAlloc *locked, u32 size)`
  - 在 alloc 已锁定的前提下更新 size

- `SetLockedStoreHead(LockedAlloc *locked, s32 store_head)`
  - 更新该 alloc 的 store 链头

- `BeginRemove(uptr base, AllocEntry *removed)`
  - 删除流程第一步：
    - 从精确哈希摘掉 alloc
    - 标成 deleting

- `RemoveRangeForEntry(const AllocEntry &entry)`
  - 删除流程第二步：
    - 从 page 范围索引中移除该 alloc

- `FinalizeRemove(s32 id)`
  - 删除流程第三步：
    - 回收 alloc 节点槽位

- `Remove(uptr base, AllocEntry *removed = nullptr)`
  - 三段删除的便捷封装

- `Size() const`
  - 当前 live alloc 数

- `Capacity() const`
  - 当前 alloc 槽位容量

- `MaxCapacity() const`
  - 最大 live alloc 上限

- `IsAtLiveCapacity() const`
  - 当前 live alloc 是否已到上限

- `BucketCount() const`
  - 精确哈希 bucket 数

- `SlabCount() const`
  - alloc node slab 数

- `BucketPageCount() const`
  - page 范围索引 bucket 数

- `StorageEntrySize() const`
  - alloc row 大小

- `BucketEntrySize() const`
  - bucket 头指针槽位大小

### 私有辅助函数说明

- `Hash(uptr value) const`
  - 通用 hash mixer

- `BaseBucket(uptr base) const`
  - 计算 `base` 落到哪个精确哈希桶

- `PageBucket(uptr page_id) const`
  - 计算 `page_id` 落到哪个 page bucket

- `PageIdForAddr(uptr addr) const`
  - 地址转 page id

- `LastPageIdForRange(uptr base, u32 size) const`
  - 计算 `[base, base+size)` 最后覆盖到的 page

- `EnsureNodeSlabPtrCapacity(uptr need_count)`
  - 确保 alloc node slab 指针数组足够大

- `EnsureNodeSlabForIndex(uptr idx)`
  - 确保包含该索引的 alloc node slab 已经物理分配

- `GrowNodeSlabs()`
  - 按 slab 粒度扩 alloc node 容量

- `GetNode(s32 idx)`
  - `id -> Node *`

- `AcquireNode()`
  - 分配一个 alloc 节点槽位

- `ReleaseNode(s32 id)`
  - 释放一个 alloc 节点槽位

- `EnsureRangeSlabPtrCapacity(uptr need_count)`
  - 确保 range-link slab 指针数组足够大

- `EnsureRangeSlabForIndex(uptr idx)`
  - 确保包含该索引的 range-link slab 已经分配

- `GrowRangeLinkSlabs()`
  - 按 slab 粒度扩 range-link 容量

- `GetRangeLink(s32 idx)`
  - `id -> RangeLink *`

- `AcquireRangeLink()`
  - 分配一个 range-link 槽位

- `ReleaseRangeLink(s32 id)`
  - 回收一个 range-link 槽位

- `FindIdInHashBucketLocked(uptr bucket, uptr base) const`
  - 在“已持有桶锁”的前提下，在桶链里找 base

- `FillEntryLocked(s32 id, const Node &node, AllocEntry *out) const`
  - 从内部 Node 填出对外 AllocEntry

- `LockNodeIfLive(s32 id, LockedAlloc *out) const`
  - 若该 alloc 仍为 live，则锁住其 `store_mu`

- `InsertRangeLinksForNode(s32 alloc_id, uptr base, u32 size)`
  - 为 alloc 覆盖到的每个 page 插一条 `RangeLink`

- `RollbackInsertedRangeLinks(...)`
  - page 链插入失败时做回滚

- `RemoveRangeLinksForNode(s32 alloc_id, uptr base, u32 size)`
  - 删除 alloc 对应的所有 page 链

---

## 4.8 `alloc_table.cpp`

文件：
- [alloc_table.cpp](/srv/workspace/memgraph/memgraph_formal_master/compiler-rt/lib/ohos_memgraph/alloc_table.cpp)

### 总体作用

实现 alloc table 的三层能力：

1. 精确哈希索引
2. page 范围索引
3. slab/free-list 存储层

### 局部静态函数

- `NextPow2(uptr value)`
  - 计算不小于输入值的最接近的 2 的幂
  - 用于 bucket 数量初始化，方便后续用按位与选桶

### 构造与基础辅助

- `AllocTable::AllocTable()`
  - 初始化所有成员为“空表状态”

- `Hash(uptr value) const`
  - hash 函数，用于打散地址值

- `BaseBucket(uptr base) const`
  - 根据 `base` 算精确哈希桶号

- `PageBucket(uptr page_id) const`
  - 根据 `page_id` 算 page bucket 号

- `PageIdForAddr(uptr addr) const`
  - 地址转页号

- `LastPageIdForRange(uptr base, u32 size) const`
  - 根据对象区间得到最后覆盖页

### alloc node slab 管理

- `EnsureNodeSlabPtrCapacity(uptr need_count)`
  - 扩容 `node_slabs_` 顶层指针数组

- `EnsureNodeSlabForIndex(uptr idx)`
  - 分配包含 `idx` 的具体 alloc node slab

- `GrowNodeSlabs()`
  - 按 slab 粒度扩 alloc 节点容量

- `GetNode(s32 idx)`
  - 稳定 id 到 `Node *` 的 O(1) 映射

- `GetNode(s32 idx) const`
  - const 版本

- `AcquireNode()`
  - 先用 free list，再用 fresh slot，再按需扩容

- `ReleaseNode(s32 id)`
  - 回收 alloc 节点槽位到 free list

### range-link slab 管理

- `EnsureRangeSlabPtrCapacity(uptr need_count)`
  - 扩容 `range_slabs_` 顶层指针数组

- `EnsureRangeSlabForIndex(uptr idx)`
  - 分配包含 `idx` 的具体 range-link slab

- `GrowRangeLinkSlabs()`
  - 按 slab 粒度扩 range-link 容量

- `GetRangeLink(s32 idx)`
  - 稳定 id 到 `RangeLink *` 的 O(1) 映射

- `GetRangeLink(s32 idx) const`
  - const 版本

- `AcquireRangeLink()`
  - 分配一条 page 链节点槽位

- `ReleaseRangeLink(s32 id)`
  - 回收一条 page 链节点

### 精确哈希内部 helper

- `FindIdInHashBucketLocked(uptr bucket, uptr base) const`
  - 在已持有精确哈希桶锁的前提下，在桶链中查找 base

- `FillEntryLocked(...)`
  - 把内部 Node 填成对外 `AllocEntry`

- `LockNodeIfLive(s32 id, LockedAlloc *out) const`
  - 如果节点仍为 live，则锁住其 `store_mu` 并导出 `LockedAlloc`

### page 范围索引内部 helper

- `InsertRangeLinksForNode(s32 alloc_id, uptr base, u32 size)`
  - 对 alloc 覆盖到的每个 page 头插一条 `RangeLink`

- `RollbackInsertedRangeLinks(...)`
  - page 链插入过程失败时回滚已插入部分

- `RemoveRangeLinksForNode(s32 alloc_id, uptr base, u32 size)`
  - 把 alloc 覆盖到的所有 page 链从范围索引中摘掉

### 生命周期

- `Init(uptr initial_capacity)`
  - 计算并初始化：
    - hash bucket 数组
    - page bucket 数组
    - bucket 锁数组
    - 容量上限

- `Destroy()`
  - 释放所有 slab、bucket 和锁数组

### 主流程函数

- `Insert(uptr base, u32 size, s32 *out_id)`
  - 新增 alloc 的主入口
  - 先拿 alloc node 槽位，再插精确哈希，再插 page 范围索引
  - 如果 page 插入失败，会回滚精确哈希和节点槽位

- `Find(uptr base, AllocEntry *out) const`
  - 精确查找 alloc

- `FindContaining(uptr addr, AllocEntry *out) const`
  - 按字段地址找 owner alloc

- `FindId(uptr base, s32 *out_id) const`
  - 精确查 id

- `GetById(s32 id, AllocEntry *out) const`
  - 按稳定 id 查 alloc 内容

- `LockByBase(uptr base, LockedAlloc *out) const`
  - 找 alloc 并锁住它自己的 `store_mu`

- `LockContaining(uptr addr, LockedAlloc *out) const`
  - 先走 page 范围索引找到 owner，再锁住 owner 的 `store_mu`

- `Unlock(LockedAlloc *locked) const`
  - 释放 owner-local store 锁

- `UpdateLockedMeta(...)`
  - 修改块级 metadata

- `UpdateLockedSize(...)`
  - 修改 size

- `SetLockedStoreHead(...)`
  - 修改 alloc 的 `store_head`

### 删除流程

- `BeginRemove(uptr base, AllocEntry *removed)`
  - 从精确哈希先摘掉 alloc，并标为 deleting

- `RemoveRangeForEntry(const AllocEntry &entry)`
  - 用 `entry.id/base/size` 把这个 alloc 从 page 索引摘掉

- `FinalizeRemove(s32 id)`
  - 把 alloc 节点清空并回收到 free list

- `Remove(uptr base, AllocEntry *removed)`
  - 组合调用三步删除

### 诊断/容量接口

- `Size() const`
  - live alloc 数

- `StorageEntrySize() const`
  - alloc row 物理大小

---

## 4.9 `store_table.h`

文件：
- [store_table.h](/srv/workspace/memgraph/memgraph_formal_master/compiler-rt/lib/ohos_memgraph/store_table.h)

### 总体作用

`StoreTable` 不再是一个“全局 store 索引表”，而是：

- 一个 **slab-backed 节点池**
- 服务于“每个 alloc 的 owner-local store 链”

### 结构体说明

- `struct StoreEntry`
  - 对外可读的 store 记录视图

- `struct StoreInfoRecordIds`
  - 诊断路径下的中间结构
  - 暂存 type_id / var_id

- `struct StoreRow`
  - store table 内部真实存储行
  - 包含：
    - `dst_offset`
    - `type_id`
    - `var_id`
    - `owner_alloc_id`
    - `next_or_free`

- `class StoreTable`
  - store 节点池与 owner-local 查找的实现主体

### 函数说明

- `StoreTable()`
  - 构造空表

- `Init(uptr capacity)`
  - 初始化最大容量

- `Destroy()`
  - 销毁全部 slab 和重置状态

- `Find(s32 head, u32 dst_offset, StoreEntry *out) const`
  - 在某个 alloc 的 store 链中按 offset 查最新记录

- `UpsertRecord(s32 owner_alloc_id, s32 head, u32 dst_offset, u32 type_id, u32 var_id)`
  - 为 owner-local 链分配一条新的历史节点，并头插到链上

- `RemoveAllForAlloc(s32 head)`
  - 清理某个 alloc 的整条 store 链

- `CountRecords(s32 head) const`
  - 统计某个 alloc 的 store 记录条数

- `GetInfoRecordIds(...) const`
  - 诊断路径下导出 store 记录的 id 信息

- `GetInfoRecords(...) const`
  - 诊断路径下导出 store 记录的字符串信息

- `Capacity() const`
  - 当前 store 槽位容量

- `MaxCapacity() const`
  - 最大 live store 上限

- `LiveCount() const`
  - 当前 live store 数

- `SlabCount() const`
  - store row slab 数

- `RowSize() const`
  - store row 物理大小

### 私有辅助函数说明

- `EnsureSlabPtrCapacity()`
  - 确保 slab 指针数组足够大

- `EnsureSlabForIndex()`
  - 确保包含某个 slot 的具体 slab 已分配

- `GrowOneSlab()`
  - 扩容一个 row slab

- `GetRow()`
  - 稳定 slot -> `StoreRow *`

- `FillEntry()`
  - 把内部 row 填成对外 `StoreEntry`

- `AcquireSlot()`
  - 分配一个 store row 槽位

- `ReleaseSlotLocked()`
  - 在已持锁前提下回收一个 row 槽位

---

## 4.10 `store_table.cpp`

文件：
- [store_table.cpp](/srv/workspace/memgraph/memgraph_formal_master/compiler-rt/lib/ohos_memgraph/store_table.cpp)

### 总体作用

实现 store 节点池。当前 formal runtime 的关键取舍是：

- 不再维护全局 store 哈希
- 不再维护全局 store FIFO
- 每个 alloc 只维护自己的 store 单链表

### 主要函数说明

- `StoreTable::StoreTable()`
  - 初始化空表状态

- `EnsureSlabPtrCapacity(uptr need_count)`
  - 扩 slab 顶层指针数组

- `EnsureSlabForIndex(uptr idx)`
  - 分配包含该 slot 的具体 row slab

- `GrowOneSlab()`
  - 按 slab 粒度扩容 store row

- `GetRow(s32 slot)`
  - 稳定 slot 到 `StoreRow *` 的映射

- `GetRow(s32 slot) const`
  - const 版本

- `FillEntry(...) const`
  - 导出可读的 `StoreEntry`

- `Init(uptr capacity)`
  - 设置容量上限

- `Destroy()`
  - 释放全部 slab 并重置状态

- `AcquireSlot()`
  - 从 free list 或 fresh slot 获取新 row
  - 超过显式容量时返回失败，触发 drop-new 语义

- `ReleaseSlotLocked(s32 slot)`
  - 清空 row 并挂回 free list

- `Find(s32 head, u32 dst_offset, StoreEntry *out) const`
  - 在某个 owner 的 store 链里扫描 offset
  - 这是 local chain 查找，不是全局查找

- `UpsertRecord(...)`
  - 总是分配一个新的历史节点，并头插到 owner 链

- `RemoveAllForAlloc(s32 head)`
  - 一次性释放 owner 链上的全部节点

- `CountRecords(s32 head) const`
  - 统计 owner 链长度

- `GetInfoRecordIds(...) const`
  - 导出 owner 链上的 type/var id

- `GetInfoRecords(...) const`
  - 导出 owner 链上的字符串记录

- `LiveCount() const`
  - 当前 live store 节点数

---

## 4.11 `memgraph.h`

文件：
- [memgraph.h](/srv/workspace/memgraph/memgraph_formal_master/compiler-rt/lib/ohos_memgraph/memgraph.h)

### 总体作用

内部总头文件，声明 runtime 的全局状态、主流程函数、查询函数和观测函数。

### 全局状态说明

- `ohos_memgraph_inited`
  - runtime 是否已经初始化

- `ohos_memgraph_init_is_running`
  - 是否正处于初始化过程

- `THREADLOCAL int ohos_memgraph_disable_interceptors`
  - 当前线程是否临时禁止 interceptor 递归进入

- `AllocTable *alloc_table`
  - alloc 主索引

- `StoreTable *store_table`
  - store 节点池

- `NameTable *type_table`
  - type 名字表

- `NameTable *var_table`
  - var 名字表

- `alloc_mu`
  - 预留/兼容全局 alloc 锁壳

- `store_mu`
  - 预留/兼容全局 store 锁壳

- `graph_mu`
  - 预留/兼容 graph 锁壳

### 函数说明

- `Initialize()`
  - 进程级初始化

- `InitializeInterceptors()`
  - 安装 malloc/free/realloc/calloc 拦截器

- `TrackHookAlloc(uptr base, uptr size)`
  - hook 路径里的 alloc 记录

- `TrackHookFree(uptr base)`
  - hook 路径里的 free 记录

- `TrackHookRealloc(uptr old_base, uptr new_base, uptr new_size)`
  - hook 路径里的 realloc 记录

- `RecordMallocMetadata(...)`
  - `alloc_record()` 的内部实现

- `RecordStoreMetadata(...)`
  - `store_record()` 的内部实现

- `GetBlockInfo(...)`
  - IDE 主查询之一：块级查询

- `GetMemberInfo(...)`
  - IDE 主查询之一：成员级查询

- `GetInfo(...)`
  - 诊断查询

- `GetInfoRecords(...)`
  - 诊断枚举查询

- `GetRuntimeStats(...)`
  - 观测快照查询

- `GetLayout(...)`
  - 布局查询

- `flags()`
  - 返回全局 flags

- `HooksEnabled()`
  - hooks 总开关

- `ObservabilityEnabled()`
  - 维测总开关

---

## 4.12 `memgraph.cpp`

文件：
- [memgraph.cpp](/srv/workspace/memgraph/memgraph_formal_master/compiler-rt/lib/ohos_memgraph/memgraph.cpp)

### 总体作用

runtime 的主控文件，负责：

- 初始化
- hook 触发后的 alloc/free/realloc 生命周期维护
- `alloc_record()` / `store_record()` 入口实现
- `get_block_info()` / `get_member_info()` 主查询实现
- flags 和环境变量处理

### 局部 helper / 类说明

- `template <class T> T *AllocateInternalObject()`
  - 用内部 allocator 创建 runtime 自己的对象

- `class ScopedDisableInterceptors`
  - 作用域内禁止 interceptor 递归进入

- `ClampSize(uptr size)`
  - 将外部 size 压到 `u32` 可表示范围内

- `ClampPositiveEnvToInt(const char *name, int fallback)`
  - 从环境变量读正整数配置

- `ClampBoolEnv(const char *name, bool fallback)`
  - 从环境变量读布尔配置

- `HasFrontendMeta(const char *value)`
  - 判断前端传来的元数据字符串是否有效

- `RefreshLiveCountersLocked()`
  - 从 authoritative tables 重新计算并发布 live counters

### flags 相关

- `Flags::SetDefaults()`
  - 读 `.inc` 默认值并套用环境变量覆盖

- `flags()`
  - 返回全局 flags 对象

- `HooksEnabled()`
  - 查看 runtime 总开关

- `ObservabilityEnabled()`
  - 查看 observability 总开关

### 生命周期函数

- `Initialize()`
  - 初始化 runtime：
    - flags
    - alloc/store/name tables
    - 观测层
    - interceptors

- `TrackHookAlloc(uptr base, uptr size)`
  - hook 分配路径：
    - 复用地址先当 free 处理
    - 新 alloc 插入 alloc table

- `TrackHookFree(uptr base)`
  - hook free 路径：
    - 从 alloc 精确哈希摘掉
    - 从 page 范围索引摘掉
    - 清理整条 store 链
    - 回收 alloc 节点

- `TrackHookRealloc(uptr old_base, uptr new_base, uptr new_size)`
  - realloc 归一化为：
    - same-address resize
    - 或 free(old) + alloc(new)

### metadata 写接口

- `RecordMallocMetadata(uptr base, const char *type_name, const char *var_name)`
  - 写块级 metadata
  - 只更新 alloc row，不涉及 store 和 page 范围索引

- `RecordStoreMetadata(uptr source_addr, uptr dst_ptr, const char *type_name, const char *var_name)`
  - 写成员级 metadata
  - 先通过 `dst_ptr` 找 owner alloc
  - 再把新 store row 头插到 owner-local 链

### IDE 主查询接口

- `GetBlockInfo(uptr base, block_info_t *out)`
  - 精确按 `base` 查询块级信息

- `GetMemberInfo(uptr base, uptr member_addr, member_info_t *out)`
  - 先按 `base` 找 owner alloc
  - 再按 offset 在 owner-local store 链中找最新记录

---

## 4.13 `memgraph_allocation_functions.cpp`

文件：
- [memgraph_allocation_functions.cpp](/srv/workspace/memgraph/memgraph_formal_master/compiler-rt/lib/ohos_memgraph/memgraph_allocation_functions.cpp)

### 总体作用

实现对外导出的 C ABI 包装层。它不承载业务逻辑，只负责：

- 保持 ABI 稳定
- 把 C 类型转换为内部 C++ 运行时调用

### 函数说明

- `alloc_record(...)`
  - 转发到 `RecordMallocMetadata()`

- `store_record(...)`
  - 转发到 `RecordStoreMetadata()`

- `get_block_info(...)`
  - 转发到 `GetBlockInfo()`

- `get_member_info(...)`
  - 转发到 `GetMemberInfo()`

- `get_info(...)`
  - 转发到 `GetInfo()`

- `get_info_records(...)`
  - 转发到 `GetInfoRecords()`

- `get_runtime_stats(...)`
  - 转发到 `GetRuntimeStats()`

- `get_layout(...)`
  - 转发到 `GetLayout()`

- `memgraph_init()`
  - 转发到 `Initialize()`

---

## 4.14 `memgraph_interceptors.cpp`

文件：
- [memgraph_interceptors.cpp](/srv/workspace/memgraph/memgraph_formal_master/compiler-rt/lib/ohos_memgraph/memgraph_interceptors.cpp)

### 总体作用

实现 `malloc/calloc/free/realloc` 拦截器，是 runtime 追踪对象生命周期的入口。

### 结构体 / 类说明

- `struct DlsymAlloc`
  - 运行时初始化阶段的临时 allocator 适配器

- `class ScopedInterceptorBypass`
  - 作用域内临时禁止 interceptor 递归进入

### 函数说明

- `BypassInterceptors()`
  - 判断当前是否应该绕过拦截器逻辑

- `INTERCEPTOR(void *, malloc, uptr size)`
  - 拦截 malloc
  - 成功分配后调用 `TrackHookAlloc()`

- `INTERCEPTOR(void *, calloc, uptr nmemb, uptr size)`
  - 拦截 calloc
  - 成功分配后调用 `TrackHookAlloc()`

- `INTERCEPTOR(void, free, void *ptr)`
  - 拦截 free
  - 先删 metadata，再调用真实 `free`

- `INTERCEPTOR(void *, realloc, void *ptr, uptr size)`
  - 拦截 realloc
  - 将行为统一归入 `TrackHookRealloc()`

- `InitializeInterceptors()`
  - 安装运行时需要的标准内存函数拦截器

---

## 4.15 `memgraph_monitoring.cpp`

文件：
- [memgraph_monitoring.cpp](/srv/workspace/memgraph/memgraph_formal_master/compiler-rt/lib/ohos_memgraph/memgraph_monitoring.cpp)

### 总体作用

承载整个 observability / diagnostics 子系统，负责：

- 运行时内存计数
- 事件计数
- `get_info()` / `get_info_records()`
- `get_runtime_stats()` / `get_layout()`
- `atexit` summary

### 结构体说明

- `struct RuntimeStatsAtomic`
  - 用原子变量维护所有观测统计
  - 这些值只用于报告，不参与功能正确性判断

### 原子 helper 说明

- `AddWithPeak(...)`
  - current 增加并更新 peak

- `SubNoUnderflow(...)`
  - current 递减但防止下溢

- `UpdateMax(...)`
  - 更新最大值

### 内部统计 hook 说明

- `MemStatsOnAllocTableAlloc/Free`
  - alloc table 内存统计

- `MemStatsOnStoreTableAlloc/Free`
  - store table 内存统计

- `MemStatsOnTypeTableAlloc/Free`
  - type table 内存统计

- `MemStatsOnVarTableAlloc/Free`
  - var table 内存统计

- `MemStatsOnMiscAlloc/Free`
  - runtime 其他对象内存统计

- `MemStatsOnMallocHookCall`
  - 记录 malloc/calloc hook 调用数

- `MemStatsOnFreeHookCall`
  - 记录 free hook 调用数

- `MemStatsOnReallocHookCall`
  - 记录 realloc hook 调用数

- `MemStatsOnMallocRecordCall`
  - 记录 `alloc_record()` 调用数

- `MemStatsOnStoreRecordCall`
  - 记录 `store_record()` 调用数

- `MemStatsUpdateLiveCounters`
  - 发布当前 live alloc/live store 数

### 诊断查询说明

- `GetInfo(uptr base, alloc_info_t *out)`
  - 返回 alloc 是否存在、大小和总记录数

- `GetInfoRecords(uptr base, info_record_t *out, uptr capacity)`
  - 枚举一个 alloc 的块级记录和成员级记录

### 快照与布局查询说明

- `MemStatsGetSnapshot(runtime_stats_t *out)`
  - 组装完整 runtime 快照

- `GetRuntimeStats(runtime_stats_t *out)`
  - 对外导出快照

- `GetLayout(unsigned long *alloc_row_bytes, unsigned long *store_row_bytes)`
  - 对外导出 alloc/store row 大小

### 进程退出摘要

- `MemStatsLogSummary()`
  - 打印进程结束时的 runtime 统计摘要

---

## 4.16 `memgraph_preinit.cpp`（已从 OHOS-only 构建中移除）

### 总体作用

这份文件原本用于通过 `.preinit_array` 机制在进程更早阶段触发 `Initialize()`。

但 `SANITIZER_CAN_USE_PREINIT_ARRAY` 在 OHOS 上默认关闭，这条路径对当前 OHOS runtime 实际不生效。由于本分支只支持 OHOS，这个源码文件已经从构建中移除，当前实际初始化路径是：

- 显式 `memgraph_init()`
- 或第一次命中接口 / 拦截器时的懒初始化 `Initialize()`

### 符号说明

- `__local_memgraph_preinit`
  - 一个放进 `.preinit_array` 的函数指针
  - 指向 `__ohos_memgraph::Initialize`

---

## 4.17 `ohos_memgraph_minimal_import.patch`

文件：
- [ohos_memgraph_minimal_import.patch](/srv/workspace/memgraph/memgraph_formal_master/compiler-rt/lib/ohos_memgraph/ohos_memgraph_minimal_import.patch)

### 总体作用

保存导入 / 迁移过程中用到的最小 patch 痕迹。它不是 runtime 主逻辑的一部分，也不参与正式库构建。

### 检视建议

- 可视为“迁移辅助材料”
- 不需要把它纳入主逻辑评审重点

---

## 5. 关键数据结构关系

### 5.1 AllocTable

逻辑上：

```text
base -> alloc
page -> candidate alloc
```

物理上：

```text
AllocTable
  |- hash_buckets_      (精确哈希桶头数组)
  |- page_buckets_      (page 范围索引桶头数组)
  |- node_slabs_        (alloc 节点 slab)
  |- range_slabs_       (page 链节点 slab)
```

### 5.2 StoreTable

逻辑上：

```text
alloc.store_head -> StoreRow -> StoreRow -> ...
```

物理上：

```text
StoreTable
  |- slabs_             (store row slab)
  |- free_head_         (复用 free list)
```

### 5.3 NameTable

逻辑上：

```text
const char * -> u32 id
u32 id -> const char *
```

---

## 6. 六条核心路径与主要函数对应关系

### 6.1 `malloc`

主调用链：

```text
malloc interceptor
  -> TrackHookAlloc()
  -> alloc_table->Insert()
```

### 6.2 `free`

主调用链：

```text
free interceptor
  -> TrackHookFree()
  -> alloc_table->BeginRemove()
  -> alloc_table->RemoveRangeForEntry()
  -> store_table->RemoveAllForAlloc()
  -> alloc_table->FinalizeRemove()
```

### 6.3 `alloc_record(base, ...)`

主调用链：

```text
alloc_record()
  -> RecordMallocMetadata()
  -> alloc_table->LockByBase()
  -> alloc_table->UpdateLockedMeta()
```

### 6.4 `store_record(dst_ptr, ...)`

主调用链：

```text
store_record()
  -> RecordStoreMetadata()
  -> alloc_table->LockContaining()
  -> store_table->UpsertRecord()
  -> alloc_table->SetLockedStoreHead()
```

### 6.5 `get_block_info(base)`

主调用链：

```text
get_block_info()
  -> GetBlockInfo()
  -> alloc_table->Find()
```

### 6.6 `get_member_info(base, member_addr)`

主调用链：

```text
get_member_info()
  -> GetMemberInfo()
  -> alloc_table->LockByBase()
  -> store_table->Find()
```

---

## 7. 测试目录走读

测试目录：
- [tests/](/srv/workspace/memgraph/memgraph_formal_master/compiler-rt/lib/ohos_memgraph/tests)

说明：
- 下面的测试说明以“文件作用 + 主要结构体 / 关键 helper / main 流程”为主
- 这些文件本身不是 runtime 主逻辑，但它们定义了当前代码的验收边界

### 7.1 功能测试

#### `memgraph_ohos_e2e_test.cpp`

文件：
- [memgraph_ohos_e2e_test.cpp](/srv/workspace/memgraph/memgraph_formal_master/compiler-rt/lib/ohos_memgraph/tests/memgraph_ohos_e2e_test.cpp)

作用：
- 覆盖 runtime 主功能端到端链路

结构体/函数：
- `RecordBuffer`
  - 暂存 `get_info_records()` 返回结果
- `LoadSym()`
  - 动态加载导出符号
- `SameString()`
  - 字符串比较 helper
- `ExpectTracked()`
  - 校验 `get_info()`
- `ExpectBlock()`
  - 校验 `get_block_info()`
- `ExpectMember()`
  - 校验 `get_member_info()`
- `LoadRecords()`
  - 拉取记录列表
- `FreeRecords()`
  - 释放临时记录缓存
- `CountRecord()`
  - 统计特定记录
- `TryReuseAddress()`
  - 验证地址复用相关行为
- `main()`
  - 组织整条 e2e 流程

#### `memgraph_edge_cases_test.cpp`

文件：
- [memgraph_edge_cases_test.cpp](/srv/workspace/memgraph/memgraph_formal_master/compiler-rt/lib/ohos_memgraph/tests/memgraph_edge_cases_test.cpp)

作用：
- 验证边界行为，如无效查询、partial metadata、same-address realloc、moved realloc

函数：
- `LoadSym()`
- `SameString()`
- `ExpectBlock()`
- `ExpectMember()`
- `VerifyPartialAndInvalidQueries()`
- `VerifySameAddressRealloc()`
- `VerifyMovedRealloc()`
- `main()`

#### `memgraph_thread_stress.cpp`

文件：
- [memgraph_thread_stress.cpp](/srv/workspace/memgraph/memgraph_formal_master/compiler-rt/lib/ohos_memgraph/tests/memgraph_thread_stress.cpp)

作用：
- 验证多线程并发下主功能路径是否稳定

结构体/函数：
- `WorkerCtx`
  - 线程上下文
- `LoadSym()`
  - 装载导出符号
- `WorkerMain()`
  - 单线程压力循环
- `main()`
  - 启动多线程并校验结果

#### `memgraph_observability_flag_smoke.cpp`

文件：
- [memgraph_observability_flag_smoke.cpp](/srv/workspace/memgraph/memgraph_formal_master/compiler-rt/lib/ohos_memgraph/tests/memgraph_observability_flag_smoke.cpp)

作用：
- 验证 `OHOS_MEMGRAPH_OBSERVABILITY_ENABLED=0` 时主功能仍工作、观测接口关闭

函数：
- `LoadSym()`
- `SameString()`
- `main()`

#### `memgraph_alloc_drop_new_test.cpp`

文件：
- [memgraph_alloc_drop_new_test.cpp](/srv/workspace/memgraph/memgraph_formal_master/compiler-rt/lib/ohos_memgraph/tests/memgraph_alloc_drop_new_test.cpp)

作用：
- 验证 alloc 达到容量上限后的 drop-new 语义

函数：
- `LoadSym()`
- `ExpectBlock()`
- `ExpectMember()`
- `main()`

#### `memgraph_store_drop_new_test.cpp`

文件：
- [memgraph_store_drop_new_test.cpp](/srv/workspace/memgraph/memgraph_formal_master/compiler-rt/lib/ohos_memgraph/tests/memgraph_store_drop_new_test.cpp)

作用：
- 验证 store 达到容量上限后的 drop-new 语义

函数：
- `LoadSym()`
- `FieldOffset()`
- `ExpectMember()`
- `main()`

### 7.2 前端 / 业务 smoke

#### `memgraph_frontend_business_block_smoke_min.cpp`

文件：
- [memgraph_frontend_business_block_smoke_min.cpp](/srv/workspace/memgraph/memgraph_formal_master/compiler-rt/lib/ohos_memgraph/tests/memgraph_frontend_business_block_smoke_min.cpp)

作用：
- 最小业务块级前端 smoke

结构体/函数：
- `struct Node`
  - 被追踪对象
- `main()`
  - 构造最小场景并验证块级元数据

#### `memgraph_frontend_business_block_member_smoke_min.cpp`

文件：
- [memgraph_frontend_business_block_member_smoke_min.cpp](/srv/workspace/memgraph/memgraph_formal_master/compiler-rt/lib/ohos_memgraph/tests/memgraph_frontend_business_block_member_smoke_min.cpp)

作用：
- 最小业务块级 + 成员级前端 smoke

结构体/函数：
- `struct Node`
- `main()`

#### `memgraph_frontend_auto_block_member_smoke_min.cpp`

文件：
- [memgraph_frontend_auto_block_member_smoke_min.cpp](/srv/workspace/memgraph/memgraph_formal_master/compiler-rt/lib/ohos_memgraph/tests/memgraph_frontend_auto_block_member_smoke_min.cpp)

作用：
- 自动插桩前提下的最小块级 + 成员级 smoke

结构体/函数：
- `struct Node`
- `main()`

#### `memgraph_frontend_integration_smoke.cpp`

文件：
- [memgraph_frontend_integration_smoke.cpp](/srv/workspace/memgraph/memgraph_formal_master/compiler-rt/lib/ohos_memgraph/tests/memgraph_frontend_integration_smoke.cpp)

作用：
- 前端联调 smoke，覆盖 block/member/info 多条查询路径

结构体/函数：
- `Node`
- `Graph`
- `LoadSym()`
- `PrintTracked()`
- `PrintBlock()`
- `PrintMember()`
- `main()`

### 7.3 benchmark

#### `memgraph_high_concurrency_bench.cpp`

作用：
- 短生命周期对象的混合全链路压力测试

结构体/函数：
- `WorkerCtx`
- `NowNs()`
- `ParseArg()`
- `LoadSym()`
- `WorkerMain()`
- `RunScenario()`
- `main()`

#### `memgraph_read_heavy_bench.cpp`

作用：
- 读路径密集场景 benchmark

结构体/函数：
- `Dataset`
- `WorkerCtx`
- `NowNs()`
- `ParseArg()`
- `LoadSym()`
- `BuildDataset()`
- `DestroyDataset()`
- `WorkerMain()`
- `RunScenario()`
- `main()`

#### `memgraph_write_hotspot_bench.cpp`

作用：
- 极端热点 owner 写场景 benchmark

结构体/函数：
- `Dataset`
- `WorkerCtx`
- `NowNs()`
- `ParseArg()`
- `LoadSym()`
- `BuildDataset()`
- `DestroyDataset()`
- `WorkerMain()`
- `RunScenario()`
- `main()`

#### `memgraph_write_steady_state_bench.cpp`

作用：
- 更贴近真实 steady-state 的写流 benchmark

结构体/函数：
- `Dataset`
- `WorkerCtx`
- `NowNs()`
- `ParseArg()`
- `LoadSym()`
- `InitializeOwner()`
- `BuildDataset()`
- `DestroyDataset()`
- `WorkerMain()`
- `RunScenario()`
- `main()`

#### `memgraph_write_spread_bench.cpp`

作用：
- 低到中等碰撞、写分布更分散的场景 benchmark

结构体/函数：
- `Dataset`
- `WorkerCtx`
- `NowNs()`
- `ParseArg()`
- `LoadSym()`
- `BuildDataset()`
- `DestroyDataset()`
- `WorkerMain()`
- `RunScenario()`
- `main()`

#### `memgraph_perf_bench.cpp`

作用：
- 汇总 alloc/store/query 吞吐和内存曲线的综合 benchmark

函数：
- `NowNs()`
- `ReadVmRssKB()`
- `ParseArg()`
- `FieldCountForSize()`
- `FieldOffset()`
- `CoveredFields()`
- `main()`

#### `memgraph_mem_curve.cpp`

作用：
- 观察 alloc/store 数量增长过程中的内存曲线

函数：
- `ReadVmRssKB()`
- `ParseArg()`
- `FieldCountForSize()`
- `FieldOffset()`
- `PrintSnapshot()`
- `main()`

### 7.4 hybrid / 兼容性测试

#### `memgraph_hybrid_frontend_block_smoke.cpp`
- 验证 hybrid 风格 block smoke

函数：
- `Node`
- `LoadSym()`
- `QueryBlock()`
- `main()`

#### `memgraph_hybrid_frontend_business_block_smoke.cpp`
- 验证 hybrid 风格业务 block smoke

函数：
- `Node`
- `LoadSym()`
- `QueryBlock()`
- `BusinessAllocateNode()`
- `BusinessBindTypedNode()`
- `main()`

### 7.5 `frontend_shared/`

#### `frontend_shared/memgraph_frontend_shared_runner.cpp`

作用：
- 通用的 shared-library smoke runner

函数：
- `main()`
  - 动态加载目标 `.so`
  - 找到指定 `run_symbol`
  - 执行测试入口

#### 其余 `*_shared.cpp`

作用：
- 与普通 frontend smoke 对应，只是改成共享库入口形式

---

## 8. 检视建议：推荐阅读顺序

如果要向别人讲解这套代码，我建议按下面顺序走：

1. [memgraph_interface.h](/srv/workspace/memgraph/memgraph_formal_master/compiler-rt/lib/ohos_memgraph/memgraph_interface.h)
   - 先讲清楚对外 ABI
2. [memgraph.h](/srv/workspace/memgraph/memgraph_formal_master/compiler-rt/lib/ohos_memgraph/memgraph.h) + [memgraph.cpp](/srv/workspace/memgraph/memgraph_formal_master/compiler-rt/lib/ohos_memgraph/memgraph.cpp)
   - 讲主流程入口
3. [alloc_table.h](/srv/workspace/memgraph/memgraph_formal_master/compiler-rt/lib/ohos_memgraph/alloc_table.h) + [alloc_table.cpp](/srv/workspace/memgraph/memgraph_formal_master/compiler-rt/lib/ohos_memgraph/alloc_table.cpp)
   - 讲 alloc 的精确哈希、范围索引、slab 存储
4. [store_table.h](/srv/workspace/memgraph/memgraph_formal_master/compiler-rt/lib/ohos_memgraph/store_table.h) + [store_table.cpp](/srv/workspace/memgraph/memgraph_formal_master/compiler-rt/lib/ohos_memgraph/store_table.cpp)
   - 讲 owner-local store 链
5. [name_table.h](/srv/workspace/memgraph/memgraph_formal_master/compiler-rt/lib/ohos_memgraph/name_table.h) + [name_table.cpp](/srv/workspace/memgraph/memgraph_formal_master/compiler-rt/lib/ohos_memgraph/name_table.cpp)
   - 讲字符串 intern
6. [memgraph_interceptors.cpp](/srv/workspace/memgraph/memgraph_formal_master/compiler-rt/lib/ohos_memgraph/memgraph_interceptors.cpp)
   - 讲 hook 生命周期接入
7. [memgraph_monitoring.cpp](/srv/workspace/memgraph/memgraph_formal_master/compiler-rt/lib/ohos_memgraph/memgraph_monitoring.cpp)
   - 讲诊断与 benchmark 支撑

---

## 9. 一句话总结

当前正式版 `ohos_memgraph` 的主设计可以压成一句：

> 用 `AllocTable` 管 live alloc 和 owner 查找，用 `StoreTable` 管每个 alloc 私有的成员记录链，用 `NameTable` 压缩字符串元数据，再通过 interceptor 和 C ABI 把 malloc/free/前端 metadata 写入与 IDE 查询串成一条可控、边界清晰、内存受限的运行时链路。
