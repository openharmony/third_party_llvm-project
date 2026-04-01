# memgraph_interceptors.cpp 逐函数代码解读

## 文件职责

`memgraph_interceptors.cpp` 是 runtime 与真实 allocator 之间的连接层。

它做两件事：

1. 用 sanitizer interception 框架拦截：
   - `malloc`
   - `calloc`
   - `free`
   - `realloc`
2. 把这些真实堆生命周期事件转发到 memgraph runtime 的内部入口：
   - `TrackHookAlloc`
   - `TrackHookFree`
   - `TrackHookRealloc`

因此这份文件决定了：

- runtime 什么时候感知到一个对象诞生
- runtime 什么时候感知到一个对象消失

---

## 相关辅助结构

### `DlsymAlloc`（`25`）

这是为初始化期准备的内部分配器适配层。

它继承自 `DlSymAllocator`，目的是：

- 当 runtime 自己还在初始化时
- 如果又需要做一些内部内存分配

不要再次走普通 allocator hook，而是走内部的安全旁路分配。

### `ScopedInterceptorBypass`（`30`）

这是一个很典型的 RAII guard。

作用：

- 构造时：`ohos_memgraph_disable_interceptors++`
- 析构时：`ohos_memgraph_disable_interceptors--`

这样在 runtime 自己处理元数据期间，hook 可以临时绕开自己，避免递归。

### `BypassInterceptors`（`38`）

作用：统一判断当前是否必须跳过 hook 包装层。

返回 true 的两类情况：

1. runtime 还在初始化，内部对象分配走 `DlsymAlloc`
2. 当前线程已经明确处于“禁用 interceptor”状态

---

## 代码主线

这份文件最好的阅读顺序是：

1. 看每个拦截器包装函数
2. 再看 `InitializeInterceptors()`

因为大部分逻辑都集中在那 4 个拦截器里。

---

## `malloc` 拦截器（`48`）

作用：拦截用户侧一次 `malloc(size)`。

逐段理解：

### `49-50`

如果当前正处在 `DlsymAlloc` 路径，就直接走内部 allocator，不再进入 memgraph 主逻辑。

### `51-52`

runtime 还没初始化时，先调用 `Initialize()`。

### `53-54`

如果：

- hooks 总开关关闭
- 或当前必须 bypass

那就直接调用真实 `malloc`，不做元数据追踪。

### `56`

调用真实 `malloc` 获得堆对象。

### `57-61`

如果分配成功：

- 进入 `ScopedInterceptorBypass`
- 调 `TrackHookAlloc(ptr, size ? size : 1)`

这里 `size == 0` 时会按 1 处理，保证 runtime 里至少有一个非零区间语义。

### `62-63`

如果 observability 开启，再记录一次 malloc hook 事件计数。

### `64`

返回真实分配到的指针。

---

## `calloc` 拦截器（`67`）

作用：拦截用户侧一次 `calloc(nmemb, size)`。

和 `malloc` 大框架一样，但多了一步乘法溢出判断。

逐段理解：

### `68-73`

这段和 `malloc` 一样：

- 初始化期走 `DlsymAlloc`
- 未初始化则 `Initialize()`
- hooks 关闭或 bypass 时直接调真实 `calloc`

### `75`

调用真实 `calloc`

### `76-80`

如果成功，并且：

- `nmemb * size` 没有溢出

那么再：

- `ScopedInterceptorBypass`
- `TrackHookAlloc(ptr, nmemb * size)`

这保证 `calloc` 进入 runtime 的对象大小语义和真实对象大小一致。

### `81-82`

观测层记录一次 malloc hook 事件。

注意这里 `calloc` 也计入 malloc hook 统计，而不是单独开一类事件。

---

## `free` 拦截器（`86`）

作用：拦截一次对象释放。

逐段理解：

### `87-88`

空指针直接返回，遵守标准 `free(nullptr)` 语义。

### `89-90`

如果指针属于 `DlsymAlloc` 自己的内部对象，就交回它自己的 `Free()`。

### `91-94`

必要时先初始化；如果 hooks 关闭或 bypass，则直接调真实 `free`。

### `96-100`

这是这条路径的关键：

- 先在 `ScopedInterceptorBypass` 下调用 `TrackHookFree(ptr)`
- 先删 metadata
- 再让真实 allocator 回收内存

顺序不能反过来，否则 runtime 可能先失去对对象地址的有效语义。

### `101-103`

观测层记录 free hook 事件，然后再调真实 `free(ptr)`。

---

## `realloc` 拦截器（`106`）

作用：统一处理 realloc 的各种语义分支。

这是最复杂的一个拦截器，因为 `realloc` 可能表现成：

- `malloc`
- `free`
- 原地 resize
- 迁移到新地址

### 第一段：`ptr == nullptr`（`107-123`）

这时语义退化成 `malloc(size)`。

流程和 `malloc` 基本一致：

- 初始化
- bypass 判断
- 调真实 `malloc`
- 成功后 `TrackHookAlloc`
- 记一次 realloc hook 事件

### 第二段：内部 allocator 指针（`125-126`）

如果当前就是内部对象或初始化期分配，直接交给 `DlsymAlloc::Realloc`。

### 第三段：正常 realloc（`127-145`）

#### `127-130`

和其他拦截器一样，先初始化，再处理 bypass。

#### `132`

调用真实 `realloc`

#### `133-142`

在 `ScopedInterceptorBypass` 保护下统一交给 `TrackHookRealloc()`。

这里又分两种：

- `size == 0`
  - 行为上按 free 处理
- `new_ptr != nullptr`
  - 交给 `TrackHookRealloc(old, new, size)`

`TrackHookRealloc()` 会在 runtime 内部进一步分流：

- 同地址 resize
- 地址变化时的 free + alloc

#### `143-145`

记录一次 realloc hook 事件并返回新指针。

---

## `InitializeInterceptors`（`152`）

作用：安装当前正式版所需的 allocator 拦截器。

逐段理解：

### `153-155`

用静态 `inited` 防止重复安装。

### `157-163`

在非 Fuchsia 平台上安装：

- `malloc`
- `calloc`
- `free`
- `realloc`

这就是 runtime 真正接入 libc allocator 的地方。

### `165`

标记安装完成。

---

## 讲解这份文件时最推荐的主线

如果你要给别人讲这份文件，建议只抓三点：

1. **为什么要有 bypass**
   - 防止 runtime 自己递归打回 hook
2. **hook 之后到底转发到哪里**
   - `TrackHookAlloc / Free / Realloc`
3. **为什么 free 一定要先删 metadata 再真实 free**

一句话概括：

> `memgraph_interceptors.cpp` 是 runtime 和真实堆分配器之间的桥，它把系统 allocator 的生命周期事件翻译成 memgraph 的生命周期模型。
