# memgraph_preinit.cpp 逐函数代码解读

> 注：这份源码已从当前 OHOS-only 构建中移除。保留这份文档是为了说明历史设计；当前实际初始化路径是显式 `memgraph_init()` 或第一次命中入口时的懒初始化 `Initialize()`。

## 文件职责

`memgraph_preinit.cpp` 很短，但时机很特殊。

它的目的不是实现新功能，而是：

- 尽量把 `Initialize()` 提前到进程启动更早阶段
- 减少“第一次真正命中 hook 或查询时才初始化”带来的时序抖动

所以这份文件回答的是：

> runtime 能不能在更早的阶段就准备好？

---

## 代码逐行解读

### 文件头和依赖（`1-14`）

- `1-11`：说明这是“更早阶段的初始化钩子”。
- `13`：引入 `memgraph.h`，因为要拿到 `__ohos_memgraph::Initialize`。
- `14`：引入 sanitizer 内部宏定义。

### `#if SANITIZER_CAN_USE_PREINIT_ARRAY`（`16`）

这行表示：

- 只有当前平台 / 工具链支持 `.preinit_array`
- 才启用这套更早初始化机制

也就是说，这不是无条件启用的。

对当前 OHOS 版本来说，这个条件默认不成立，所以这条早初始化路径实际上不会启用。

### `__local_memgraph_preinit`（`17-19`）

这是整份文件真正的核心。

它做的事情是：

1. 定义一个函数指针变量
2. 把它放进 `.preinit_array` 段
3. 指向 `__ohos_memgraph::Initialize`

这样在程序正常进入 `main` 之前，运行时加载阶段就有机会先调用 `Initialize()`。

逐行理解：

- `17`
  - 用 `section(".preinit_array")` 指定这个变量进入 `.preinit_array`
  - `used` 防止链接器优化掉它
- `18-19`
  - 把这个函数指针赋值为 `&__ohos_memgraph::Initialize`

---

## `.preinit_array` 到底是什么，执行时机在哪里

`.preinit_array` 是 ELF 程序里的一个特殊段，用来存放“进程正式进入业务代码前就要调用的函数指针”。

可以把它理解成：

- 一个超早期初始化函数列表
- 比普通全局构造和常规初始化段还要更早

在常见 ELF 启动流程里，执行顺序可以粗略理解成：

1. 加载主程序和依赖库
2. 执行 `.preinit_array`
3. 执行 `.init_array`
4. 再进入 `main`

所以，这里把：

- `__ohos_memgraph::Initialize`

注册进 `.preinit_array`，就可以理解成：

- **尽量在进程启动超早阶段执行一次 memgraph runtime 初始化**

也就是说，你前面那句理解是对的：

> 相当于在 `.preinit_array` 段就执行这个运行时的初始化函数 `__ohos_memgraph::Initialize`

更准确一点说，是：

- 在进程启动时，loader 处理 `.preinit_array` 时，会调用这里登记进去的函数指针
- 而这个函数指针正好指向 `__ohos_memgraph::Initialize`

所以最终效果就是：

- `Initialize()` 尽量早于普通业务代码
- 也尽量早于第一次真正命中 malloc hook / 前端 metadata 写入 / IDE 查询

---

## 为什么这个文件只有几行却很重要

因为它改变的不是“逻辑复杂度”，而是“初始化时机”。

有了它之后，runtime 更容易在：

- 第一次用户侧 `malloc`
- 第一次前端 `alloc_record/store_record`
- 第一次 IDE 查询

之前就准备好自身状态。

好处是：

- 减少首个调用承担初始化成本
- 降低懒初始化下的时序不确定性

---

## 讲解这份文件时怎么讲

最简单的讲法就是：

> 这份文件什么都不“算”，它只负责把 `Initialize()` 提前挂到进程更早的启动阶段。

如果对方问“为什么只有一个函数指针”，你就解释：

- `.preinit_array` 本来就是靠“把函数指针放进特殊段”来实现的
- 这里不是少写了逻辑，而是机制本来就这么简单直接

---

## 一句话总结

`memgraph_preinit.cpp` 的全部价值在于：

- **把 runtime 初始化从“第一次使用时”尽量前移到“进程启动时”**。
