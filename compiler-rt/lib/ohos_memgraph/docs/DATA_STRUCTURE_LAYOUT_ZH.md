# OHOS Memgraph 数据结构布局图

这份文档把当前 formal 版 memgraph 里最核心的 4 组数据结构单独画出来，方便在代码检视、设计讲解和新人 onboarding 时直接使用。

覆盖的 4 张图是：

1. `AllocTable` 整体布局
2. owner-local `store` 链布局
3. page 范围索引哈希表布局
4. `NameTable` 布局

---

## 1. AllocTable 整体布局

相关代码：

- [alloc_table.h](/srv/workspace/memgraph/memgraph_formal_master/compiler-rt/lib/ohos_memgraph/alloc_table.h)
- [alloc_table.cpp](/srv/workspace/memgraph/memgraph_formal_master/compiler-rt/lib/ohos_memgraph/alloc_table.cpp)

`AllocTable` 不是单一结构，而是三层组合：

- 精确哈希：`base -> alloc`
- page 范围索引：`page_id -> candidate allocs`
- slab / free-list 存储层

```text
AllocTable
|
+-- 精确哈希表: base -> alloc
|   |
|   +-- hash_buckets_[bucket] = alloc_id
|   |       |
|   |       v
|   |   Node(hash_next_or_free) -> Node -> Node ...
|   |
|   +-- hash_mutexes_[bucket]
|
+-- page 范围索引: page_id -> candidate allocs
|   |
|   +-- page_buckets_[bucket] = range_link_id
|   |       |
|   |       v
|   |   RangeLink(next_or_free) -> RangeLink -> RangeLink ...
|   |
|   +-- page_mutexes_[bucket]
|
+-- alloc 节点池
|   |
|   +-- node_slabs_[slab_id] -> Node[4096]
|   +-- free_head_
|   +-- next_node_
|
+-- page 索引节点池
    |
    +-- range_slabs_[slab_id] -> RangeLink[4096]
    +-- range_free_head_
    +-- next_range_link_
```

### 这张图要点

- `hash_buckets_` 负责按 `base` 精确找 alloc。
- `page_buckets_` 负责按 `dst_ptr` 所在页反查 owner。
- `node_slabs_` 和 `range_slabs_` 只负责“节点住在哪”，不负责查找语义。

---

## 2. owner-local store 链布局

相关代码：

- [store_table.h](/srv/workspace/memgraph/memgraph_formal_master/compiler-rt/lib/ohos_memgraph/store_table.h)
- [store_table.cpp](/srv/workspace/memgraph/memgraph_formal_master/compiler-rt/lib/ohos_memgraph/store_table.cpp)

当前 formal 版里没有全局 store 哈希表。  
每个 alloc 自己挂一条 store 链。

```text
Alloc Node #37
  base = 0x0000007f12345020
  size = 64
  store_head = 20000
                |
                v
            StoreRow #20000
                |
                +-- owner_alloc_id = 37
                +-- dst_offset = 16
                +-- type_id = 20
                +-- var_id = 8
                +-- next_or_free = 52
                                  |
                                  v
                              StoreRow #52
                                  |
                                  +-- owner_alloc_id = 37
                                  +-- dst_offset = 8
                                  +-- type_id = 19
                                  +-- var_id = 7
                                  +-- next_or_free = -1
```

### `store_head` 到真实 `StoreRow` 的映射

```text
Node.store_head
    |
    v
store slot id
    |
    +--> slab_id = store_slot_id / kRowsPerSlab
    |
    +--> row_id  = store_slot_id % kRowsPerSlab
    |
    v
store_slabs_[slab_id][row_id]
    |
    v
真正的 StoreRow
```

### 这张图要点

- `store_head` 存的是 **store slot id**，不是裸指针，也不是 slab 号。
- 每条 `StoreRow` 存的是“某个 owner 的某个 offset 的一条 metadata 历史记录”。
- `next_or_free` 在 live 时是链表 next，在 free 时复用成 free-list next。

---

## 3. page 范围索引哈希表布局

相关代码：

- [alloc_table.h](/srv/workspace/memgraph/memgraph_formal_master/compiler-rt/lib/ohos_memgraph/alloc_table.h)
- [alloc_table.cpp](/srv/workspace/memgraph/memgraph_formal_master/compiler-rt/lib/ohos_memgraph/alloc_table.cpp)

这张表不是按 `base` 查，而是按：

- `page_id = addr >> 12`

查“这一页上的 alloc 候选集合”。

### 总体结构

```text
dst_ptr
  |
  +--> page_id = dst_ptr >> 12
  |
  v
PageBucket(page_id)
  |
  v
page_buckets_[bucket]
  |
  v
RangeLink -> RangeLink -> RangeLink ...
```

### `RangeLink` 表达的语义

```text
RangeLink
+---------------------------+
| page_id                   |  这条记录属于哪一页
| alloc_id                  |  这一页上的候选 alloc
| next_or_free              |  live时=页桶链next, free时=free-list next
+---------------------------+
```

### 真实例子

假设：

- `dst_ptr = 0x0000007f12345038`

那么：

```text
page_id = 0x0000007f12345038 >> 12
        = 0x00000007f12345
```

page 哈希表里这一个 key 对应的 value 不是单个 alloc，而是候选集：

```text
0x00000007f12345 -> [alloc #52, alloc #37]
```

实现上会是：

```text
page_buckets_[bucket(page_id)]
  -> RangeLink(page_id, alloc_id=52)
  -> RangeLink(page_id, alloc_id=37)
```

接着 runtime 再逐个取 `alloc_id`，通过 `GetNode(alloc_id)` 找到真实 `Node`，做最终判断：

```text
base <= dst_ptr < base + size
```

命中的那个 alloc 才是真正 owner。

### 这张图要点

- `key = page_id`
- `value = 这一页上的 alloc 候选链`
- page 索引不能直接唯一确定 owner，它只是把范围缩小到“这一页上的候选集”

---

## 4. NameTable 布局

相关代码：

- [name_table.h](/srv/workspace/memgraph/memgraph_formal_master/compiler-rt/lib/ohos_memgraph/name_table.h)
- [name_table.cpp](/srv/workspace/memgraph/memgraph_formal_master/compiler-rt/lib/ohos_memgraph/name_table.cpp)

`NameTable` 负责把：

- `const char *type_name`
- `const char *var_name`

压成：

- `u32 type_id`
- `u32 var_id`

它内部不是链式哈希，而是：

- 开放定址哈希表
- 再加一张 `id -> ptr` 数组

```text
NameTable
|
+-- map_ : Slot[map_cap]
|    |
|    +-- Slot {
|          ptr,   // 原始 const char *
|          id,    // 压缩后的 u32 id
|          state
|        }
|
+-- id_to_ptr_ : const char *[id_cap]
```

### 两条映射关系

#### 正向：`ptr -> id`

```text
const char *ptr
   |
   v
Hash(ptr)
   |
   v
map_[slot]   (开放定址探测)
   |
   v
u32 id
```

#### 反向：`id -> ptr`

```text
u32 id
  |
  v
id_to_ptr_[id]
  |
  v
const char *ptr
```

### 这张图要点

- `NameTable` 当前按“指针身份”去重，不按字符串内容去重。
- 内部实际上有两套结构：
  - `ptr -> id`
  - `id -> ptr`
- alloc/store 表内部真正存的是 `u32 id`，不直接存字符串指针。

---

## 5. 四张图串起来的整体关系

如果把这 4 张图放在一条主路径里看，当前 formal 版的 runtime 模型就是：

```text
alloc_record(base, type, var)
  -> AllocTable 精确哈希(base -> alloc)
  -> 更新 Alloc Node 里的块级 metadata

store_record(dst_ptr, type, var)
  -> page 哈希表(page_id -> candidate allocs)
  -> 找到 owner alloc
  -> 用 owner.store_head 进入 owner-local store 链
  -> 头插新的 StoreRow

type_name / var_name
  -> NameTable(ptr -> id)
  -> alloc/store 节点内部只存 u32 id
```

也就是说：

- `AllocTable` 负责找到“对象”
- page 哈希表负责从“字段地址”反查对象
- store 链负责挂“字段级 metadata 历史记录”
- `NameTable` 负责把字符串压成整数 id

---

## 6. 最短总结

如果你要现场快速概括这四张图，可以直接这么说：

- `AllocTable` 管对象本身。
- page 哈希表解决“字段地址属于哪个对象”。
- store 链管对象下面的成员 metadata。
- `NameTable` 负责把字符串压缩成 id。

这四层叠起来，就是当前 formal 版 memgraph 的核心数据模型。
