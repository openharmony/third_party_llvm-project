# OHOS Memgraph Data Structure Layout

This document isolates the 4 core data-structure groups in the current formal memgraph runtime so they can be reused directly during code review, design walkthroughs, and onboarding.

The 4 diagrams covered here are:

1. `AllocTable` overall layout
2. owner-local `store` chain layout
3. page range index hash-table layout
4. `NameTable` layout

---

## 1. `AllocTable` Overall Layout

Relevant code:

- [alloc_table.h](/srv/workspace/memgraph/memgraph_formal_master/compiler-rt/lib/ohos_memgraph/alloc_table.h)
- [alloc_table.cpp](/srv/workspace/memgraph/memgraph_formal_master/compiler-rt/lib/ohos_memgraph/alloc_table.cpp)

`AllocTable` is not a single flat structure. It is a combination of three layers:

- exact hash index: `base -> alloc`
- page range index: `page_id -> candidate allocs`
- slab / free-list storage layer

```text
AllocTable
|
+-- exact hash table: base -> alloc
|   |
|   +-- hash_buckets_[bucket] = alloc_id
|   |       |
|   |       v
|   |   Node(hash_next_or_free) -> Node -> Node ...
|   |
|   +-- hash_mutexes_[bucket]
|
+-- page range index: page_id -> candidate allocs
|   |
|   +-- page_buckets_[bucket] = range_link_id
|   |       |
|   |       v
|   |   RangeLink(next_or_free) -> RangeLink -> RangeLink ...
|   |
|   +-- page_mutexes_[bucket]
|
+-- alloc node pool
|   |
|   +-- node_slabs_[slab_id] -> Node[4096]
|   +-- free_head_
|   +-- next_node_
|
+-- page-index node pool
    |
    +-- range_slabs_[slab_id] -> RangeLink[4096]
    +-- range_free_head_
    +-- next_range_link_
```

### Key points

- `hash_buckets_` is used for exact alloc lookup by `base`.
- `page_buckets_` is used to recover the owner alloc from a field address.
- `node_slabs_` and `range_slabs_` only answer “where nodes live”; they do not define lookup semantics.

---

## 2. owner-local `store` Chain Layout

Relevant code:

- [store_table.h](/srv/workspace/memgraph/memgraph_formal_master/compiler-rt/lib/ohos_memgraph/store_table.h)
- [store_table.cpp](/srv/workspace/memgraph/memgraph_formal_master/compiler-rt/lib/ohos_memgraph/store_table.cpp)

The current formal runtime does not keep a global store hash table.
Each alloc owns its own store chain.

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

### How `store_head` maps to the real `StoreRow`

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
real StoreRow
```

### Key points

- `store_head` stores a **store slot id**, not a raw pointer and not a slab id.
- Each `StoreRow` represents one metadata-history record for one offset under one owner.
- `next_or_free` is reused: list-next while live, free-list-next after recycle.

---

## 3. page Range Index Hash Table Layout

Relevant code:

- [alloc_table.h](/srv/workspace/memgraph/memgraph_formal_master/compiler-rt/lib/ohos_memgraph/alloc_table.h)
- [alloc_table.cpp](/srv/workspace/memgraph/memgraph_formal_master/compiler-rt/lib/ohos_memgraph/alloc_table.cpp)

This table is not indexed by `base`. It is indexed by:

- `page_id = addr >> 12`

and is used to answer “which allocs are candidates on this page”.

### Overall structure

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

### Meaning of `RangeLink`

```text
RangeLink
+---------------------------+
| page_id                   |  page covered by this link
| alloc_id                  |  candidate alloc on this page
| next_or_free              |  bucket next while live, free-list next after recycle
+---------------------------+
```

### Real example

Assume:

- `dst_ptr = 0x0000007f12345038`

Then:

```text
page_id = 0x0000007f12345038 >> 12
        = 0x00000007f12345
```

In the page hash table, this key maps not to one alloc but to a candidate set:

```text
0x00000007f12345 -> [alloc #52, alloc #37]
```

In the actual implementation this may look like:

```text
page_buckets_[bucket(page_id)]
  -> RangeLink(page_id, alloc_id=52)
  -> RangeLink(page_id, alloc_id=37)
```

The runtime then resolves each `alloc_id` through `GetNode(alloc_id)` and performs the final containment test:

```text
base <= dst_ptr < base + size
```

The matching alloc is the real owner.

### Key points

- `key = page_id`
- `value = candidate alloc chain on that page`
- the page index does not uniquely identify the owner; it narrows the search to the candidate set on one page

---

## 4. `NameTable` Layout

Relevant code:

- [name_table.h](/srv/workspace/memgraph/memgraph_formal_master/compiler-rt/lib/ohos_memgraph/name_table.h)
- [name_table.cpp](/srv/workspace/memgraph/memgraph_formal_master/compiler-rt/lib/ohos_memgraph/name_table.cpp)

`NameTable` compresses:

- `const char *type_name`
- `const char *var_name`

into:

- `u32 type_id`
- `u32 var_id`

Internally it is not a chained hash table. It is:

- an open-addressing hash table
- plus an `id -> ptr` array

```text
NameTable
|
+-- map_ : Slot[map_cap]
|    |
|    +-- Slot {
|          ptr,   // original const char *
|          id,    // compressed u32 id
|          state
|        }
|
+-- id_to_ptr_ : const char *[id_cap]
```

### Two mapping directions

#### Forward: `ptr -> id`

```text
const char *ptr
   |
   v
Hash(ptr)
   |
   v
map_[slot]   (open-address probing)
   |
   v
u32 id
```

#### Reverse: `id -> ptr`

```text
u32 id
  |
  v
id_to_ptr_[id]
  |
  v
const char *ptr
```

### Key points

- `NameTable` currently deduplicates by pointer identity, not by string content.
- Internally it keeps two structures:
  - `ptr -> id`
  - `id -> ptr`
- alloc/store tables only store `u32 id` values, not raw string pointers.

---

## 5. How the 4 Diagrams Fit Together

If we place the 4 diagrams on one execution path, the current formal runtime model is:

```text
alloc_record(base, type, var)
  -> AllocTable exact hash (base -> alloc)
  -> update block-level metadata in the Alloc Node

store_record(dst_ptr, type, var)
  -> page hash table (page_id -> candidate allocs)
  -> find the owner alloc
  -> enter the owner-local store chain through owner.store_head
  -> head-insert a new StoreRow

type_name / var_name
  -> NameTable(ptr -> id)
  -> alloc/store nodes store only u32 ids
```

That means:

- `AllocTable` finds the object itself.
- the page hash table maps a field address back to an object.
- the store chain holds field-level metadata history under that object.
- `NameTable` compresses strings into integer ids.

---

## 6. Short Summary

If you need to summarize these 4 diagrams quickly during a live walkthrough, this is the shortest version:

- `AllocTable` manages the objects themselves.
- the page hash table answers “which object owns this field address”.
- the store chain manages member metadata under each object.
- `NameTable` compresses strings into ids.

Together, these 4 layers form the core data model of the current formal memgraph runtime.
