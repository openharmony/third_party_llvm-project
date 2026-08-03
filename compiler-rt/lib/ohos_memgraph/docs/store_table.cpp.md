# store_table.cpp 逐函数代码解读

## 文件职责

`store_table.cpp` 是 formal 版 memgraph 里 **owner-local store 节点池** 的实现。

它的定位和旧版 V2 的“全局 store 表”不同：

- 当前版本里，每个 alloc 自己有一条 store 单链表。
- `StoreTable` 不负责全局查找 owner。
- `StoreTable` 只负责：
  - store 节点的物理存储
  - slot 分配与回收
  - 在某个 owner 的本地链里按 offset 查找最新记录
  - 把整条 owner-local 链转成诊断输出

所以它更像一个“节点池 + 链表辅助工具”，而不是一张全局 store 索引表。

---

## 相关结构体

这些结构定义在 `store_table.h`。

### `StoreEntry`（`store_table.h:30`）

这是对外可返回的 store 记录快照，字段包括：

- `dst_offset`：字段相对 owner base 的偏移。
- `type_id / var_id`：名字表中的压缩 id。
- `owner_alloc_id`：它属于哪个 alloc。
- `id`：这条 store 记录自己的稳定 id。

### `StoreInfoRecordIds`（`42`）

这是诊断路径里的轻量中间结构，只保存：

- `type_id`
- `var_id`

用于先把 id 列出来，再交给 `NameTable` 解码成字符串。

### `StoreRow`（`48`）

这是内部真实存储行。

关键字段：

- `dst_offset`
- `type_id`
- `var_id`
- `owner_alloc_id`
- `next_or_free`
  - live 时：owner-local 链的 next
  - free 时：free-list 的 next

它和 `AllocTable::Node` 的设计思路一致，都是“链表 next”和“free-list next”复用同一个字段。

---

## 代码主线

这份文件可以分成 5 段：

1. slab 指针数组扩容
2. slab 真实分配
3. slot 分配 / 回收
4. owner-local 链上的查找 / 写入 / 删除
5. 诊断导出

---

## 1. 构造和存储层

### `StoreTable::StoreTable`（`25`）

作用：把整张 store 节点池初始化为空状态。

做的事包括：

- `slabs_ = nullptr`
- `slab_count_ = 0`
- `capacity_ = 0`
- `next_slot_ = 0`
- `free_head_ = -1`
- `live_count_ = 0`

这一步不分配真正的 slab。

### `EnsureSlabPtrCapacity`（`35`）

作用：确保顶层 `slabs_` 指针数组足够大。

逐段理解：

- 如果当前 `slab_ptr_cap_` 已经够，就直接返回。
- 否则申请更大的指针数组。
- 把旧的 slab 指针拷过去。
- 新数组替换旧数组。

它扩的是“入口数组”，不是 store row 真正存储块。

### `EnsureSlabForIndex`（`62`）

作用：确保某个 slot 所在的 slab 已经被真实分配。

流程：

1. 计算 `slab_idx = idx / kRowsPerSlab`
2. 必要时先扩 slab 指针数组
3. 如果 `slabs_[slab_idx]` 还没分配，就申请一整块 `StoreRow[kRowsPerSlab]`
4. 初始化新 slab 里的 `next_or_free`

### `GrowOneSlab`（`87`）

作用：一次扩一整块 store slab。

它会：

- 确保 `capacity_` 对应的下一块 slab 存在
- 然后把 `capacity_` 加上一整个 `kRowsPerSlab`

### `GetRow`（`97` / `110 const`）

作用：把稳定 slot id 映射回真实 `StoreRow *`。

流程和 alloc 的 `GetNode()` 一样：

1. `slab_idx = slot / kRowsPerSlab`
2. `row_idx = slot % kRowsPerSlab`
3. 返回对应地址

### `FillEntry`（`124`）

作用：把内部 `StoreRow` 转成外部 `StoreEntry`。

它只做字段拷贝，不做链表遍历或查找。

---

## 2. 生命周期

### `Init`（`136`）

作用：初始化 store 节点池的最大容量。

注意：

- `max_capacity_` 是 live store 数上限。
- slab 仍然是按需增长。

### `Destroy`（`142`）

作用：释放所有 store slabs，并把整张表恢复成空状态。

会重置：

- `slabs_`
- `slab_count_`
- `capacity_`
- `next_slot_`
- `free_head_`
- `live_count_`

---

## 3. slot 分配 / 回收

### `AcquireSlot`（`170`）

作用：拿一个新的可用 store slot。

优先级：

1. 先从 free-list 复用。
2. free-list 为空时，从未使用区域拿新 slot。
3. 容量不够时 `GrowOneSlab()`。

逐段理解：

- `171-176`：先看 free-list 头。
- `177-186`：必要时增长容量，再从尾部拿 `next_slot_`。
- `187-188`：失败时返回 `-1`。

### `ReleaseSlotLocked`（`191`）

作用：把一个 slot 回收到 free-list。

因为这个函数名里带 `Locked`，表示调用者应当已经持有 `pool_mu_`。

做的事：

- 清空该行的语义字段
- 用 `next_or_free` 串回 `free_head_`
- live 计数减一

---

## 4. owner-local 链操作

### `Find`（`209`）

作用：在某个 owner 的 store 链里，按 `dst_offset` 查找“最新记录”。

当前写入策略是：

- 新记录头插

所以查找时只要从链头往后扫：

- 第一条 `dst_offset` 匹配的记录
- 就是这个 offset 的最新值

逐段理解：

- 从 `head` 开始沿 `next_or_free` 遍历。
- 每拿到一行，就比较：
  - `row->dst_offset == dst_offset`
- 命中后 `FillEntry()` 返回。

### `UpsertRecord`（`228`）

作用：为某个 owner 新建一条 store 历史记录，并头插到链表前端。

名字里虽然叫 `Upsert`，但当前语义不是“更新旧节点”，而是：

- 始终新建一条历史节点
- 再头插

完整流程：

1. 持有 `pool_mu_`
2. `AcquireSlot()` 拿到新 slot
3. 填好这条 `StoreRow`
4. `next_or_free = old_head`
5. 返回新的链头 slot

所以调用方收到返回值之后，需要把 owner alloc 的 `store_head` 改成这个新 slot。

### `RemoveAllForAlloc`（`252`）

作用：清理某个 alloc 的整条 store 链。

流程：

1. 从 `head` 开始遍历
2. 先保存 `next`
3. 在 `pool_mu_` 保护下调用 `ReleaseSlotLocked()`
4. 继续下一条

这通常在 `free(base)` 路径里由上层调用。

---

## 5. 诊断接口

### `CountRecords`（`271`）

作用：统计某条 owner-local 链的节点数。

它只沿链表数节点个数，不解析名字。

### `GetInfoRecordIds`（`284`）

作用：把 owner-local 链里的记录转成一组 `type_id / var_id`。

它本身不解码字符串，只负责把 id 填出来。

### `GetInfoRecords`（`302`）

作用：把 owner-local 链转成对外的 `info_record_t[]`。

流程：

1. 遍历链表
2. 对每条记录用：
   - `type_table->Resolve(type_id)`
   - `var_table->Resolve(var_id)`
3. 填 `info_record_t`

### `LiveCount`（`322`）

作用：返回当前 live store 数。

这是一个观测接口，不参与功能主路径判定。

---

## 现场讲解这份文件时怎么讲

建议主线是：

1. **先讲 store 表已经不是全局索引了**
2. **再讲它现在只做节点池**
3. **再讲 owner-local 链的写入是头插**
4. **最后讲诊断接口只是把链转回字符串**

一句话概括就是：

> `StoreTable` 现在不是“我怎么找 owner”，而是“owner 已经找到了之后，我怎么给它挂 store 节点、怎么查最新记录、怎么把整条链清掉”。
