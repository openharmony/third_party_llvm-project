# name_table.cpp 逐函数代码解读

## 文件职责

`name_table.cpp` 实现的是 memgraph 的名字压缩表。

它负责把前端传进来的：

- `const char *type_name`
- `const char *var_name`

压成较小的：

- `u32 type_id`
- `u32 var_id`

这样 alloc/store 表内部就不用反复存完整指针字符串，而只保存整数 id。

这张表当前的去重语义不是“按字符串内容”，而是：

- **按字符串指针身份**

也就是说，只有当前端保证传进来的名字指针是稳定静态字符串时，这套压缩才成立。

---

## 相关结构体

### `NameTableKind`（`name_table.h:25`）

作用：区分这张名字表到底是：

- `kTypeNameTable`
- 还是 `kVarNameTable`

这样内存统计时，type table 和 var table 可以分别记账。

### `NameTable::Slot`（`name_table.h:53`）

这是内部 open-addressing 哈希表的槽位。

字段含义：

- `ptr`
  - 原始字符串指针
- `id`
  - 分配给它的稳定整数 id
- `state`
  - 槽位状态（是否已占用）

这说明 `NameTable` 的底层不是链式哈希，而是开放定址。

---

## 代码主线

`name_table.cpp` 可以按下面顺序理解：

1. 内存统计 helper
2. hash 和 rehash
3. `id -> ptr` 数组扩容
4. 初始化与销毁
5. `Intern` 和 `Resolve`

---

## 1. 统计 helper

### `NameTable::NameTable`（`31`）

作用：构造一个空名字表。

会把：

- `map_`
- `id_to_ptr_`
- `map_cap_`
- `id_cap_`
- `id_size_`

等成员都置成初始空状态。

### `OnAlloc`（`36`）

作用：把名字表占用的内存记到对应统计项。

根据 `kind_` 判断：

- 是 type table 就记到 type 统计
- 是 var table 就记到 var 统计

### `OnFree`（`44`）

作用：和 `OnAlloc` 对偶，把对应内存从统计里减掉。

---

## 2. 哈希与重建

### `Hash`（`52`）

作用：根据字符串指针地址计算 hash 值。

关键点：

- 这里 hash 的是“指针值”
- 不是字符串内容

所以它速度很快，但语义前提也更强。

### `Rehash`（`66`）

作用：把内部开放定址哈希表重建到一个新的容量。

完整流程：

1. 分配新的 `Slot[]`
2. 把所有新槽位清空
3. 遍历旧 map
4. 对每个已占用槽重新计算 hash
5. 用线性探测重新插入到新 map
6. 释放旧 map
7. 更新 `map_`、`map_cap_`

这一步的目标是：

- 降低装载因子
- 保持 `Intern()` 的平均探测长度较短

---

## 3. `id -> ptr` 数组扩容

### `EnsureIdCapacity`（`100`）

作用：确保 `id_to_ptr_` 这个数组能放下下一个新 id。

为什么需要这层结构：

- `Intern(ptr)` 需要返回 `u32 id`
- 但 `Resolve(id)` 又要能反查原始 `const char *`

所以除了哈希表，还需要一张：

- `id -> ptr`

数组。

这个函数做的事是：

1. 看 `id_size_ + 1` 是否还能装进 `id_cap_`
2. 不够就申请更大的数组
3. 拷贝旧内容
4. 替换旧数组

---

## 4. 生命周期

### `Init`（`126`）

作用：初始化名字表类型和底层存储。

主要工作：

- 记录 `kind_`
- 初始化内部 map 容量
- 初始化 `id_to_ptr_` 容量

### `Destroy`（`134`）

作用：释放：

- 哈希表 `map_`
- `id_to_ptr_` 数组

并重置容量和计数。

---

## 5. 核心接口

### `Intern`（`156`）

作用：把字符串指针压成稳定 `u32 id`。

这是整个文件最核心的函数。

完整流程：

1. 先处理空指针：
   - 通常把空值映射为 `0`
2. 持有 `mu_`
3. 如果装载因子过高，先 `Rehash()`
4. 计算 `Hash(ptr)`
5. 在开放定址哈希表里做线性探测：
   - 找到同样的 `ptr`：直接返回已有 id
   - 找到空槽位：准备插入
6. 确保 `id_to_ptr_` 容量够
7. 分配一个新的 `id`
8. 往：
   - `map_`
   - `id_to_ptr_`

同时写入
9. 返回新 id

为什么它不是按字符串内容去重：

- 因为根本没有 `strcmp`
- 比较的是指针相等

### `Resolve`（`188`）

作用：把 `id` 反查回原始字符串指针。

逻辑很轻：

- 检查 `id` 是否越界
- 直接 `return id_to_ptr_[id]`

这就是为什么 `EnsureIdCapacity()` 那条数组很重要。

### `MapEntrySize`（`196`）

作用：返回内部哈希表一个槽位的物理大小。

主要用于：

- 监控
- `get_layout`
- benchmark 估算

---

## 怎么讲这份文件

这份文件很适合用一句话概括：

> `NameTable` 做的是“把稳定字符串指针 intern 成整数 id，再支持 id 反查原始字符串”。

如果现场讲，建议顺序：

1. 先说明它按**指针身份**去重，不按字符串内容。
2. 再讲它内部有两张结构：
   - `ptr -> id` 的开放定址哈希表
   - `id -> ptr` 的数组
3. 最后讲：
   - `Intern()` 负责正向压缩
   - `Resolve()` 负责反查展开

听众会很容易明白为什么 alloc/store 里只存 `u32` 而不存字符串指针。
