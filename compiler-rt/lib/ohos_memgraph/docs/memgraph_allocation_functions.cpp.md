# memgraph_allocation_functions.cpp 逐函数代码解读

## 文件职责

`memgraph_allocation_functions.cpp` 是 **对外 C ABI 包装层**。

这份文件本身非常薄，不承载核心逻辑。它做的事情只有一件：

- 保持稳定的 C 接口符号名
- 再把参数转发给内部 C++ runtime

因此你可以把它理解成：

- ABI 边界文件
- 而不是功能实现文件

---

## 为什么这份文件单独存在

原因主要有两个：

1. 对外需要稳定的 C 符号：
   - `alloc_record`
   - `store_record`
   - `get_block_info`
   - `get_member_info`
   - ...
2. 内部实现希望保留在 `__ohos_memgraph` 命名空间下，用 C++ 组织

所以这里就是一个“壳”。

---

## 文件结构

这份文件分三段：

1. 主功能导出接口
2. 诊断 / 观测导出接口
3. 显式初始化接口

---

## 1. 主功能导出接口

### `alloc_record`（`23-29`）

作用：对外暴露 `alloc_record` C ABI。

逐段理解：

- `23`：`SANITIZER_INTERFACE_ATTRIBUTE` 表示这是对外导出接口。
- `24-25`：C 形态函数签名。
- `26-28`：把 `unsigned long malloc_addr` 转成内部 `uptr`，然后转发给：
  - `__ohos_memgraph::RecordMallocMetadata(...)`

这层没有额外逻辑，也不做重写。

### `store_record`（`31-38`）

作用：对外暴露 `store_record` C ABI。

逐段理解：

- `32-33`：C ABI 形态接收：
  - `source_addr`
  - `dst_ptr`
  - `type_name`
  - `var_name`
- `34`：注释明确指出，当前 formal 版里 `source_addr` 只是 ABI 兼容保留字段。
- `35-37`：把参数转给：
  - `RecordStoreMetadata(source_addr, dst_ptr, ...)`

### `get_block_info`（`40-44`）

作用：把内部布尔返回值转成对外 `int`。

逐段理解：

- 调内部 `GetBlockInfo(base, out)`
- 成功返回 `1`
- 失败返回 `0`

### `get_member_info`（`46-53`）

作用：和 `get_block_info` 同构，只是换成成员级查询。

---

## 2. 诊断 / 观测导出接口

### `get_info`（`59-62`）

作用：导出 alloc 摘要查询接口。

行为模式和前面的查询一致：

- 内部 `bool`
- 外部 `int`

### `get_info_records`（`64-69`）

作用：导出记录枚举接口。

这里返回值本身就是数量，所以直接把内部 `uptr` 转成 `unsigned long`。

### `get_runtime_stats`（`71-74`）

作用：导出 runtime 统计快照查询。

### `get_layout`（`76-82`）

作用：导出 alloc/store 行大小查询。

对 benchmark 和内存模型观测有用。

---

## 3. 显式初始化接口

### `memgraph_init`（`88-89`）

作用：给外部一个显式初始化入口。

它只做：

- `__ohos_memgraph::Initialize()`

虽然 runtime 默认是懒初始化的，但这个接口可以让测试或工具显式控制初始化时机。

---

## 怎么讲这份文件

讲这份文件时不要把时间花在“业务逻辑”上，因为这里几乎没有业务逻辑。

最适合的讲法是：

1. **这是 ABI 边界**
2. **外面看到的是 C 函数名**
3. **里面真正干活的是 `memgraph.cpp` 里的 C++ 实现**

一句话概括：

> `memgraph_allocation_functions.cpp` 是对外壳层，不是运行时主逻辑。
