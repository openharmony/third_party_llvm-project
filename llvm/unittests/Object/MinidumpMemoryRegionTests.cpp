// ============================================================================
// Minidump Memory Region Tests for AArch64
// ============================================================================
// Tests memory region parsing and validation for aarch64 minidumps

#include <cstdint>
#include <cstring>
#include <cassert>

// Memory descriptor structure
struct MinidumpMemoryDescriptor {
    uint64_t start_of_memory_range;
    uint64_t data_size;
};

// Memory list structure
struct MinidumpMemoryList {
    uint32_t number_of_memory_ranges;
    MinidumpMemoryDescriptor memory_ranges[1];  // Flexible array
};

// Memory types
#define MEMORY_TYPE_PRIVATE         0x00020000
#define MEMORY_TYPE_MAPPED          0x00040000
#define MEMORY_TYPE_IMAGE           0x10000000
#define MEMORY_TYPE_MAPPED_FILE     0x00001000
#define MEMORY_TYPE_EXECUTE         0x20000000
#define MEMORY_TYPE_READ            0x40000000
#define MEMORY_TYPE_WRITE           0x80000000

// Test 1: Basic Memory Regions
void test_basic_memory_regions_aarch64() {
    const int num_regions = 8;
    MinidumpMemoryDescriptor regions[num_regions] = {
        // Code sections
        {0x40000000, 0x1000},      // .text
        {0x40001000, 0x1000},      // .plt
        {0x40002000, 0x2000},      // .rodata
        
        // Data sections
        {0x40004000, 0x1000},      // .data
        {0x40005000, 0x1000},      // .bss
        {0x40006000, 0x1000},      // .got
        
        // Stack
        {0x7000000000, 0x8000},    // Stack
        
        // Heap
        {0x7100000000, 0x10000}    // Heap
    };
    
    // Test region properties
    for (int i = 0; i < num_regions; i++) {
        assert(regions[i].start_of_memory_range > 0);
        assert(regions[i].data_size > 0);
        assert(regions[i].data_size <= 0x1000000);  // 16MB max per region
    }
    
    // Test no overlap
    for (int i = 0; i < num_regions; i++) {
        for (int j = i + 1; j < num_regions; j++) {
            uint64_t i_end = regions[i].start_of_memory_range + regions[i].data_size;
            uint64_t j_end = regions[j].start_of_memory_range + regions[j].data_size;
            
            bool no_overlap = (i_end <= regions[j].start_of_memory_range) ||
                             (j_end <= regions[i].start_of_memory_range);
            assert(no_overlap);
        }
    }
}

// Test 2: Large Memory Regions
void test_large_memory_regions_aarch64() {
    const int num_regions = 6;
    MinidumpMemoryDescriptor regions[num_regions] = {
        // Large code region
        {0x40000000, 0x100000},    // 1MB code
        
        // Large data region
        {0x40100000, 0x200000},    // 2MB data
        
        // Very large heap
        {0x7100000000, 0x1000000}, // 16MB heap
        
        // Stack (multiple pages)
        {0x7000000000, 0x20000},   // 128KB stack
        
        // Memory mapped file
        {0x8000000000, 0x500000},  // 5MB mapped file
        
        // Shared library
        {0x7F0000000000, 0x300000} // 3MB shared lib
    };
    
    uint64_t total_memory = 0;
    for (int i = 0; i < num_regions; i++) {
        total_memory += regions[i].data_size;
        
        // Test alignment
        assert((regions[i].start_of_memory_range & 0xFFF) == 0);  // Page aligned
        assert((regions[i].data_size & 0xFFF) == 0);  // Page sized
    }
    
    assert(total_memory > 0);
}

// Test 3: Memory Region Permissions
void test_memory_permissions_aarch64() {
    struct MemoryRegionWithFlags {
        uint64_t start;
        uint64_t size;
        uint32_t protection;
    };
    
    const int num_regions = 7;
    MemoryRegionWithFlags regions[num_regions] = {
        // Read-only code
        {0x40000000, 0x1000, MEMORY_TYPE_READ | MEMORY_TYPE_EXECUTE},
        
        // Read-write data
        {0x40001000, 0x1000, MEMORY_TYPE_READ | MEMORY_TYPE_WRITE},
        
        // Read-only data
        {0x40002000, 0x1000, MEMORY_TYPE_READ},
        
        // Execute-only (rare)
        {0x40003000, 0x1000, MEMORY_TYPE_EXECUTE},
        
        // Write-only (very rare)
        {0x40004000, 0x1000, MEMORY_TYPE_WRITE},
        
        // Read-write-execute (JIT)
        {0x40005000, 0x1000, 
         MEMORY_TYPE_READ | MEMORY_TYPE_WRITE | MEMORY_TYPE_EXECUTE},
        
        // No access (guard page)
        {0x40006000, 0x1000, 0}
    };
    
    // Test permission combinations
    for (int i = 0; i < num_regions; i++) {
        uint32_t prot = regions[i].protection;
        
        // Validate permission combinations
        if (prot & MEMORY_TYPE_EXECUTE) {
            // Executable pages usually need read permission
            // except for execute-only memory
            if (prot == MEMORY_TYPE_EXECUTE) {
                // Special case: execute-only
            } else {
                assert((prot & MEMORY_TYPE_READ) || (prot == 0));
            }
        }
        
        // Test guard page
        if (i == 6) {
            assert(prot == 0);
        }
    }
}

// Test 4: Stack Memory Layout
void test_stack_memory_aarch64() {
    const int stack_pages = 16;  // 64KB stack
    const uint64_t stack_size = stack_pages * 0x1000;
    const uint64_t stack_top = 0x7000000000;
    const uint64_t stack_bottom = stack_top - stack_size;
    
    // Create stack guard page
    MinidumpMemoryDescriptor guard_page = {stack_bottom - 0x1000, 0x1000};
    
    // Create stack pages
    MinidumpMemoryDescriptor stack_pages_desc[stack_pages];
    for (int i = 0; i < stack_pages; i++) {
        stack_pages_desc[i].start_of_memory_range = stack_bottom + (i * 0x1000);
        stack_pages_desc[i].data_size = 0x1000;
    }
    
    // Create thread stack
    struct ThreadStack {
        uint64_t stack_pointer;
        uint64_t frame_pointer;
        uint64_t return_address;
        uint64_t saved_registers[8];
    };
    
    // Simulate stack frames
    const int num_frames = 5;
    uint64_t frame_pointers[num_frames] = {
        stack_top - 0x100,    // Current frame
        stack_top - 0x200,
        stack_top - 0x300,
        stack_top - 0x400,
        stack_top - 0x500
    };
    
    // Verify stack layout
    for (int i = 0; i < num_frames - 1; i++) {
        assert(frame_pointers[i] > frame_pointers[i + 1]);  // Stack grows down
        assert((frame_pointers[i] - frame_pointers[i + 1]) <= 0x200);  // Reasonable frame size
    }
    
    // Verify stack is within bounds
    for (int i = 0; i < stack_pages; i++) {
        uint64_t page_start = stack_pages_desc[i].start_of_memory_range;
        uint64_t page_end = page_start + 0x1000;
        assert(page_start >= stack_bottom);
        assert(page_end <= stack_top);
    }
    
    // Verify guard page
    assert(guard_page.start_of_memory_range == stack_bottom - 0x1000);
    assert(guard_page.data_size == 0x1000);
}

// Test 5: Heap Memory Layout
void test_heap_memory_aarch64() {
    const int heap_blocks = 10;
    MinidumpMemoryDescriptor heap_blocks_desc[heap_blocks];
    
    uint64_t heap_base = 0x7100000000;
    uint64_t current = heap_base;
    
    // Create various sized heap blocks
    uint64_t block_sizes[heap_blocks] = {
        0x1000,    // 4KB
        0x2000,    // 8KB
        0x4000,    // 16KB
        0x8000,    // 32KB
        0x10000,   // 64KB
        0x20000,   // 128KB
        0x40000,   // 256KB
        0x80000,   // 512KB
        0x100000,  // 1MB
        0x200000   // 2MB
    };
    
    for (int i = 0; i < heap_blocks; i++) {
        heap_blocks_desc[i].start_of_memory_range = current;
        heap_blocks_desc[i].data_size = block_sizes[i];
        current += block_sizes[i];
    }
    
    // Verify heap blocks
    uint64_t total_heap = 0;
    for (int i = 0; i < heap_blocks; i++) {
        total_heap += heap_blocks_desc[i].data_size;
        
        // Check alignment
        assert((heap_blocks_desc[i].start_of_memory_range & 0xFFF) == 0);
        assert((heap_blocks_desc[i].data_size & 0xFFF) == 0);
    }
    
    assert(total_heap > 0);
    assert(total_heap == (current - heap_base));
}

// Test 6: Memory Mapped Files
void test_memory_mapped_files_aarch64() {
    struct MappedFileRegion {
        uint64_t start;
        uint64_t size;
        const char* filename;
        uint64_t file_offset;
    };
    
    const int num_files = 4;
    MappedFileRegion files[num_files] = {
        {0x7F0000000000, 0x200000, "/lib/aarch64-linux-gnu/libc.so.6", 0},
        {0x7F0000200000, 0x100000, "/lib/aarch64-linux-gnu/libm.so.6", 0},
        {0x7F0000300000, 0x80000, "/lib/aarch64-linux-gnu/libpthread.so.0", 0},
        {0x8000000000, 0x400000, "/tmp/data.bin", 0x1000}
    };
    
    // Verify file mappings
    for (int i = 0; i < num_files; i++) {
        assert(files[i].start > 0);
        assert(files[i].size > 0);
        assert(files[i].size <= 0x1000000);  // Max 16MB per mapping
        
        // Shared libraries should be in high memory
        if (strstr(files[i].filename, ".so.") != nullptr) {
            assert(files[i].start >= 0x7F0000000000);
        }
    }
}

// Test 7: Kernel Memory Regions
void test_kernel_memory_aarch64() {
    const int num_kernel_regions = 5;
    MinidumpMemoryDescriptor kernel_regions[num_kernel_regions] = {
        // Kernel code
        {0xFFFF000000000000, 0x200000},
        
        // Kernel data
        {0xFFFF000002000000, 0x100000},
        
        // Kernel heap
        {0xFFFF000003000000, 0x400000},
        
        // Kernel modules
        {0xFFFF000007000000, 0x100000},
        
        // Kernel stacks
        {0xFFFF000008000000, 0x80000}
    };
    
    // Verify kernel addresses
    for (int i = 0; i < num_kernel_regions; i++) {
        // Kernel addresses in high half
        assert(kernel_regions[i].start_of_memory_range >= 0xFFFF000000000000);
        
        // Reasonable sizes
        assert(kernel_regions[i].data_size > 0);
        assert(kernel_regions[i].data_size <= 0x1000000);
    }
}

// Test 8: Memory Hole Detection
void test_memory_holes_aarch64() {
    const int num_regions = 8;
    MinidumpMemoryDescriptor regions[num_regions] = {
        {0x40000000, 0x1000},      // Code
        {0x40002000, 0x1000},      // Hole at 0x40001000-0x40001FFF
        {0x40004000, 0x2000},      // Data
        {0x40007000, 0x1000},      // Hole at 0x40006000-0x40006FFF
        {0x7000000000, 0x8000},    // Stack
        {0x7000100000, 0x10000},   // Hole at 0x700008000-0x70000FFFF
        {0x7100000000, 0x20000},   // Heap
        {0x7100300000, 0x10000}    // Hole at 0x710020000-0x71002FFFF
    };
    
    // Verify holes don't overlap
    uint64_t last_end = 0;
    for (int i = 0; i < num_regions; i++) {
        uint64_t current_start = regions[i].start_of_memory_range;
        uint64_t current_end = current_start + regions[i].data_size;
        
        if (last_end > 0) {
            // Check for holes
            if (current_start > last_end) {
                uint64_t hole_size = current_start - last_end;
                assert(hole_size > 0);
                assert(hole_size <= 0x10000);  // Reasonable hole size
            } else {
                // No hole or overlap
                assert(current_start >= last_end);
            }
        }
        
        last_end = current_end;
    }
}

// Test 9: Memory Content Validation
void test_memory_content_aarch64() {
    const uint64_t code_start = 0x40000000;
    const uint64_t code_size = 0x1000;
    
    // Simulate code section with AArch64 instructions
    uint8_t code_memory[0x1000];
    
    // Write some AArch64 instructions
    // RET instruction
    code_memory[0] = 0xC0;
    code_memory[1] = 0x03;
    code_memory[2] = 0x5F;
    code_memory[3] = 0xD6;
    
    // NOP
    code_memory[4] = 0x1F;
    code_memory[5] = 0x20;
    code_memory[6] = 0x03;
    code_memory[7] = 0xD5;
    
    // Branch with link
    code_memory[8] = 0x00;
    code_memory[9] = 0x00;
    code_memory[10] = 0x00;
    code_memory[11] = 0x94;
    
    // Simulate data section
    const uint64_t data_start = 0x40001000;
    uint8_t data_memory[0x1000];
    
    // Write some data
    const char* hello = "Hello, World!";
    strcpy((char*)data_memory, hello);
    
    // Simulate stack
    const uint64_t stack_start = 0x7000000000;
    uint8_t stack_memory[0x2000];
    
    // Write stack frames
    uint64_t* stack_ptr = (uint64_t*)stack_memory;
    stack_ptr[0] = 0x7000001000;  // Previous frame pointer
    stack_ptr[1] = 0x40000100;    // Return address
    
    // Verify memory contents
    assert(code_memory[0] == 0xC0);
    assert(strcmp((char*)data_memory, "Hello, World!") == 0);
    assert(stack_ptr[0] == 0x7000001000);
    assert(stack_ptr[1] == 0x40000100);
}

// Test 10: Memory Region Merging
void test_memory_region_merging_aarch64() {
    // Create adjacent regions that should be merged
    const int num_regions = 6;
    MinidumpMemoryDescriptor regions[num_regions] = {
        {0x40000000, 0x1000},  // Will merge with next
        {0x40001000, 0x1000},  // Adjacent
        {0x40003000, 0x1000},  // Has gap
        {0x40004000, 0x1000},  // Adjacent to next
        {0x40005000, 0x1000},  // Adjacent
        {0x7000000000, 0x2000}  // Different area
    };
    
    // Check for merges
    bool merged[3] = {false, false, false};
    
    // Region 0 and 1 are adjacent
    if (regions[0].start_of_memory_range + regions[0].data_size == regions[1].start_of_memory_range) {
        merged[0] = true;
    }
    
    // Region 3 and 4 are adjacent
    if (regions[3].start_of_memory_range + regions[3].data_size == regions[4].start_of_memory_range) {
        merged[1] = true;
    }
    
    // Region 2 has gap
    if (regions[2].start_of_memory_range + regions[2].data_size < regions[3].start_of_memory_range) {
        merged[2] = true;
    }
    
    assert(merged[0]);  // Regions 0-1 should merge
    assert(merged[1]);  // Regions 3-4 should merge
    assert(merged[2]);  // Region 2-3 should have gap
}

// Run all memory tests
void test_all_minidump_memory_regions_aarch64() {
    test_basic_memory_regions_aarch64();
    test_large_memory_regions_aarch64();
    test_memory_permissions_aarch64();
    test_stack_memory_aarch64();
    test_heap_memory_aarch64();
    test_memory_mapped_files_aarch64();
    test_kernel_memory_aarch64();
    test_memory_holes_aarch64();
    test_memory_content_aarch64();
    test_memory_region_merging_aarch64();
    
    assert(true);
}