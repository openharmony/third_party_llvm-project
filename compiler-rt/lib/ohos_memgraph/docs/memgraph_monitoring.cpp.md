# memgraph_monitoring.cpp 逐函数代码解读

## 文件职责

`memgraph_monitoring.cpp` 集中实现 runtime 的 **观测侧逻辑**。

它不决定主功能正确性，而是回答下面这些问题：

- runtime 当前用了多少内存？
- 历史峰值是多少？
- 当前有多少 live alloc / live store？
- hook / `alloc_record` / `store_record` 分别被调用了多少次？
- 某个 alloc 当前挂了多少条 metadata？
- 退出进程时要不要打印一份摘要？

因此这份文件最好被理解成：

- **监控层**
- **诊断层**
- **benchmark 辅助层**

而不是主功能模型本身。

---

## 先看内部状态

### `RuntimeStatsAtomic`（`44`）

这是 observability 背后的原子计数集合。

它把数据分成几大类：

- runtime 总内存当前值 / 峰值
- alloc/store/type/var/misc 各组件当前值 / 峰值
- live alloc / live store 当前值 / 峰值
- hook / metadata-write 事件计数

关键理解：

- 这些计数只是“报告值”
- 主功能不会依赖这些值做正确性判断

### `runtime_stats`（`71`）

这是这组原子计数的全局单例。

---

## 代码主线

这份文件可以分成 4 段：

1. 基础原子更新 helper
2. `MemStatsOn*` 观测 hook
3. 对外 observability 查询
4. atexit summary

---

## 1. 基础原子 helper

### `AddWithPeak`（`74`）

作用：给某个 current 计数加上 `delta`，同时维护 peak。

逐段理解：

- `76-77`：先原子加上 `delta`，得到更新后的 `now`
- `78-81`：如果 `now > old_peak`，用 CAS 把 peak 往上推

它适用于所有“当前值上涨，并可能刷新峰值”的场景。

### `SubNoUnderflow`（`85`）

作用：原子递减 current，但绝不下溢到负数。

逐段理解：

- 先读旧值 `old`
- 实际扣减量取 `min(old, delta)`
- CAS 更新到 `next`

这能保证即使多线程重复 free，也不会把观测计数减成一个很大的无符号数。

### `UpdateMax`（`96`）

作用：如果新值更大，就把某个 max 原子更新到更大值。

它比 `AddWithPeak` 更轻，只负责“拿大者”。

---

## 2. `MemStatsOn*` 观测 hook

这一组函数都很短，设计原则是：

- 热路径可调用
- 但主功能绝不依赖这些值

### 组件内存记账

#### `MemStatsOnAllocTableAlloc`（`112`）

作用：alloc table 占用内存增加时记账。

做的事：

- alloc current / peak 增加
- runtime 总 current / peak 也同步增加

#### `MemStatsOnAllocTableFree`（`120`）

作用：alloc table 释放内存时记账。

做的事：

- alloc current 减少
- runtime 总 current 减少

#### `MemStatsOnStoreTableAlloc/Free`（`127` / `135`）

和 alloc table 那组同构，只是对象换成了 store table。

#### `MemStatsOnTypeTableAlloc/Free`（`142` / `150`）

记录 type 名字表的内存变化。

#### `MemStatsOnVarTableAlloc/Free`（`157` / `165`）

记录 var 名字表的内存变化。

#### `MemStatsOnMiscAlloc/Free`（`172` / `180`）

记录那些不属于 alloc/store/type/var 四大表的杂项 runtime 内存。

### 事件计数

#### `MemStatsOnMallocHookCall`（`187`）
#### `MemStatsOnFreeHookCall`（`193`）
#### `MemStatsOnReallocHookCall`（`199`）
#### `MemStatsOnMallocRecordCall`（`205`）
#### `MemStatsOnStoreRecordCall`（`212`）

这几条都很直接：

- observability 关闭时直接返回
- 否则对应的事件计数原子加一

### live 计数刷新

#### `MemStatsUpdateLiveCounters`（`219`）

作用：把 authoritative 的 live alloc / live store 数发布给监控层。

逐段理解：

- `222-225`：刷新 current
- `226-227`：刷新 peak

注意这里并不自己去数表，而是接受外面已经算好的 live 数。

---

## 3. 对外 observability 查询

### `GetInfo`（`239`）

作用：把某个 alloc 的摘要信息组装成 `alloc_info_t`。

完整流程：

1. 参数校验，`out` 为空直接失败。
2. 清空输出结构。
3. runtime 未初始化或 observability 关闭则失败。
4. `alloc_table->LockByBase(base, &entry)` 找到并锁住 alloc。
5. 填：
   - `base`
   - `size`
6. 统计逻辑记录数：
   - alloc 自己如果有块级 metadata，算 1 条
   - 再加上 store 链条数
7. 标记 `found = 1`
8. 解锁并返回

### `GetInfoRecords`（`262`）

作用：把某个 alloc 挂着的全部 metadata 记录导出为 `info_record_t[]`。

这是本文件里最复杂的一个查询函数。

完整流程：

1. 若 runtime 未初始化或 observability 关闭，直接返回 0。
2. 锁住目标 alloc。
3. 先数出 store 链长度。
4. 计算本次最多能复制多少条 store 记录。
5. 如果需要，先分配一块临时 `StoreInfoRecordIds[]`。
6. 调 `store_table->GetInfoRecordIds()` 把链上的 `type_id / var_id` 抽出来。
7. 解锁 alloc。
8. 结果输出顺序固定：
   - 先 alloc 自己的块级 metadata
   - 再 store 链上的成员级记录
9. 通过 `type_table->Resolve()` / `var_table->Resolve()` 把 id 反解成字符串。
10. 释放临时数组，返回总记录数。

这里要注意：

- 返回值是“总逻辑记录数”
- 即使输出 buffer 容量不够，也会返回完整数量

这对 benchmark 和测试很有用。

---

## 4. runtime 快照与布局

### `MemStatsGetSnapshot`（`328`）

作用：组装完整 `runtime_stats_t` 快照。

逐段理解：

1. 参数校验并清空输出。
2. observability 关闭则失败。
3. 从 `runtime_stats` 原子计数里拷贝：
   - 各种 current / peak
   - 事件计数
4. 再直接从 live runtime 对象里读取结构型信息：
   - alloc capacity
   - alloc slab 数
   - alloc bucket 数
   - page bucket 数
   - store capacity
   - store slab 数

为什么后半段不从原子里读？

- 因为这些不是热路径计数，而是表的当前形状属性。

### `GetRuntimeStats`（`395`）

作用：一个很薄的包装：

- observability 关闭则失败
- 否则直接调用 `MemStatsGetSnapshot()`

### `GetLayout`（`401`）

作用：导出当前 alloc row 和 store row 的物理大小。

流程：

1. 参数校验
2. runtime 必须已初始化
3. observability 必须开启
4. 从：
   - `alloc_table->StorageEntrySize()`
   - `store_table->RowSize()`

读取并返回

---

## 5. 退出阶段 summary

### `MemStatsLogSummary`（`418`）

作用：在进程退出前打印一份 runtime 摘要。

完整流程：

1. observability 关闭则直接返回
2. 拿一份 `runtime_stats_t` 快照
3. 暂时增加 `ohos_memgraph_disable_interceptors`
   - 避免打印过程本身再递归触发 hook
4. 打三行 summary：
   - runtime 总体内存 + live 峰值 + capacity
   - alloc/store/type/var 细分 + slab/bucket 形状
   - hook / metadata-write 事件数
5. 恢复 interceptor 状态

这份打印完全是诊断输出，不参与主功能。

---

## 讲解这份文件时最推荐的主线

你可以把它概括成两层：

1. **热路径里非常小的记账 hook**
2. **查询时再把这些分散计数打包成人类可读快照**

一句话总结：

> `memgraph_monitoring.cpp` 负责“让我们看见 runtime 自己在做什么”，而不是“决定 runtime 主功能应该怎么做”。
