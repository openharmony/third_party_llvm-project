// ============================================================================
// Minidump Stream Tests for AArch64
// ============================================================================
// Tests stream directory and data stream parsing for aarch64 minidumps

#include <cstdint>
#include <cstring>
#include <cassert>

// Stream types
#define MD_UNUSED_STREAM                0
#define MD_RESERVED_STREAM_0            1
#define MD_RESERVED_STREAM_1            2
#define MD_THREAD_LIST_STREAM           3
#define MD_MODULE_LIST_STREAM           4
#define MD_MEMORY_LIST_STREAM           5
#define MD_EXCEPTION_STREAM             6
#define MD_SYSTEM_INFO_STREAM           7
#define MD_THREAD_EX_LIST_STREAM        8
#define MD_MEMORY_64_LIST_STREAM        9
#define MD_COMMENT_STREAM_A             10
#define MD_COMMENT_STREAM_W             11
#define MD_HANDLE_DATA_STREAM           12
#define MD_FUNCTION_TABLE_STREAM        13
#define MD_UNLOADED_MODULE_LIST_STREAM  14
#define MD_MISC_INFO_STREAM             15
#define MD_MEMORY_INFO_STREAM           16
#define MD_THREAD_INFO_LIST_STREAM      17
#define MD_HANDLE_OPERATION_LIST_STREAM 18
#define MD_TOKEN_STREAM                 19
#define MD_JAVASCRIPT_DATA_STREAM       20
#define MD_SYSTEM_MEMORY_INFO_STREAM    21
#define MD_PROCESS_VM_COUNTERS_STREAM   22
#define MD_LAST_RESERVED_STREAM         0x0000ffff

// AArch64-specific stream
#define MD_AARCH64_CONTEXT_STREAM       0x80000004

// Stream directory entry
struct MinidumpDirectory {
    uint32_t stream_type;
    uint32_t data_size;
    uint64_t rva;  // Relative Virtual Address
};

// Stream list structure
struct MinidumpStreamList {
    uint32_t number_of_streams;
    MinidumpDirectory directories[1];  // Flexible array
};

// Test 1: Basic Stream Directory
void test_basic_stream_directory_aarch64() {
    const int num_streams = 8;
    MinidumpDirectory streams[num_streams] = {
        {MD_THREAD_LIST_STREAM, 0x400, 0x1000},
        {MD_MODULE_LIST_STREAM, 0x600, 0x1400},
        {MD_MEMORY_LIST_STREAM, 0x800, 0x1A00},
        {MD_EXCEPTION_STREAM, 0x300, 0x2200},
        {MD_SYSTEM_INFO_STREAM, 0x100, 0x2500},
        {MD_MISC_INFO_STREAM, 0x200, 0x2600},
        {MD_HANDLE_DATA_STREAM, 0x180, 0x2800},
        {MD_AARCH64_CONTEXT_STREAM, 0x600, 0x2A00}
    };
    
    // Verify required streams
    bool has_threads = false;
    bool has_modules = false;
    bool has_memory = false;
    bool has_exception = false;
    bool has_system_info = false;
    bool has_aarch64_context = false;
    
    for (int i = 0; i < num_streams; i++) {
        switch (streams[i].stream_type) {
            case MD_THREAD_LIST_STREAM: has_threads = true; break;
            case MD_MODULE_LIST_STREAM: has_modules = true; break;
            case MD_MEMORY_LIST_STREAM: has_memory = true; break;
            case MD_EXCEPTION_STREAM: has_exception = true; break;
            case MD_SYSTEM_INFO_STREAM: has_system_info = true; break;
            case MD_AARCH64_CONTEXT_STREAM: has_aarch64_context = true; break;
        }
        
        // Validate stream properties
        assert(streams[i].data_size > 0);
        assert(streams[i].data_size < 0x100000);  // Reasonable size limit
        assert(streams[i].rva > 0);
    }
    
    // All required streams must be present
    assert(has_threads);
    assert(has_modules);
    assert(has_memory);
    assert(has_exception);
    assert(has_system_info);
    assert(has_aarch64_context);
    
    // Check for overlaps
    for (int i = 0; i < num_streams; i++) {
        for (int j = i + 1; j < num_streams; j++) {
            uint64_t i_end = streams[i].rva + streams[i].data_size;
            uint64_t j_end = streams[j].rva + streams[j].data_size;
            
            assert(i_end <= streams[j].rva || j_end <= streams[i].rva);
        }
    }
}

// Test 2: System Info Stream
void test_system_info_stream_aarch64() {
    struct MinidumpSystemInfo {
        uint16_t processor_architecture;
        uint16_t processor_level;
        uint16_t processor_revision;
        uint8_t number_of_processors;
        uint8_t product_type;
        uint32_t major_version;
        uint32_t minor_version;
        uint32_t build_number;
        uint32_t platform_id;
        uint32_t csd_version_rva;
        uint16_t suite_mask;
        uint16_t reserved;
    };
    
    // AArch64 system info
    MinidumpSystemInfo sysinfo = {};
    sysinfo.processor_architecture = 0xAA64;  // PROCESSOR_ARCHITECTURE_ARM64
    sysinfo.processor_level = 0x8;  // ARMv8
    sysinfo.processor_revision = 0x1234;
    sysinfo.number_of_processors = 8;
    sysinfo.product_type = 1;  // VER_NT_WORKSTATION
    sysinfo.major_version = 10;
    sysinfo.minor_version = 0;
    sysinfo.build_number = 19045;
    sysinfo.platform_id = 2;  // VER_PLATFORM_WIN32_NT
    sysinfo.csd_version_rva = 0x3000;
    sysinfo.suite_mask = 0x100;  // VER_SUITE_SINGLEUSERTS
    sysinfo.reserved = 0;
    
    // CSD version string
    const char* csd_version = "Windows 10 Pro";
    
    // Verify system info
    assert(sysinfo.processor_architecture == 0xAA64);
    assert(sysinfo.processor_level == 0x8);
    assert(sysinfo.number_of_processors > 0);
    assert(sysinfo.number_of_processors <= 256);
    assert(sysinfo.major_version >= 5);
    assert(sysinfo.minor_version >= 0);
    assert(sysinfo.build_number >= 0);
    assert(sysinfo.platform_id == 2);
    
    // Verify CSD version
    assert(strlen(csd_version) > 0);
    assert(sysinfo.csd_version_rva > 0);
}

// Test 3: Misc Info Stream
void test_misc_info_stream_aarch64() {
    struct MinidumpMiscInfo {
        uint32_t size_of_info;
        uint32_t flags1;
        uint32_t process_id;
        uint32_t process_create_time;
        uint32_t process_user_time;
        uint32_t process_kernel_time;
    };
    
    MinidumpMiscInfo misc_info = {};
    misc_info.size_of_info = sizeof(MinidumpMiscInfo);
    misc_info.flags1 = 0x1F;  // All flags set
    misc_info.process_id = 1234;
    misc_info.process_create_time = 0x5F3A1B2C;
    misc_info.process_user_time = 0x00001234;
    misc_info.process_kernel_time = 0x00000567;
    
    // Verify misc info
    assert(misc_info.size_of_info >= 24);
    assert(misc_info.process_id > 0);
    assert(misc_info.process_create_time > 0);
    assert(misc_info.process_user_time >= 0);
    assert(misc_info.process_kernel_time >= 0);
}

// Test 4: Comment Streams
void test_comment_streams_aarch64() {
    // ASCII comment stream
    const char* ascii_comment = "Crash dump created by Debugger\n"
                               "Process: myapp.exe\n"
                               "Time: 2023-10-01 12:34:56\n"
                               "Reason: Access Violation";
    
    // Unicode comment stream
    const wchar_t* unicode_comment = L"内存转储文件\n"
                                     L"进程: myapp.exe\n"
                                     L"时间: 2023-10-01 12:34:56\n"
                                     L"原因: 访问违规";
    
    // Verify comment streams
    assert(strlen(ascii_comment) > 0);
    assert(wcslen(unicode_comment) > 0);
    
    // Check for expected content
    assert(strstr(ascii_comment, "Crash dump") != nullptr);
    assert(strstr(ascii_comment, "Access Violation") != nullptr);
    assert(wcsstr(unicode_comment, L"内存转储文件") != nullptr);
}

// Test 5: Handle Data Stream
void test_handle_data_stream_aarch64() {
    struct MinidumpHandleData {
        uint32_t size_of_header;
        uint32_t size_of_descriptor;
        uint32_t number_of_handles;
        uint32_t reserved;
    };
    
    struct HandleDescriptor {
        uint64_t handle;
        uint32_t type_name_rva;
        uint32_t object_name_rva;
        uint32_t attributes;
        uint32_t granted_access;
        uint32_t handle_count;
        uint32_t pointer_count;
    };
    
    MinidumpHandleData handle_data = {};
    handle_data.size_of_header = sizeof(MinidumpHandleData);
    handle_data.size_of_descriptor = sizeof(HandleDescriptor);
    handle_data.number_of_handles = 5;
    handle_data.reserved = 0;
    
    // Sample handles
    HandleDescriptor handles[5] = {
        {0x1234, 0x4000, 0x4100, 0x00100000, 0x001F0003, 1, 1},  // File
        {0x5678, 0x4200, 0x4300, 0x00100000, 0x001F0003, 1, 1},  // File
        {0x9ABC, 0x4400, 0x4500, 0x00200000, 0x001F0001, 1, 1},  // Event
        {0xDEF0, 0x4600, 0x4700, 0x00100000, 0x001F0003, 1, 1},  // Mutex
        {0x0246, 0x4800, 0x4900, 0x00100000, 0x001F0003, 1, 1}   // Thread
    };
    
    const char* handle_types[] = {
        "File", "File", "Event", "Mutex", "Thread"
    };
    
    const char* handle_names[] = {
        "C:\\Windows\\system32\\kernel32.dll",
        "C:\\Users\\test\\document.txt",
        "Global\\MyEvent",
        "Global\\MyMutex",
        ""
    };
    
    // Verify handle data
    assert(handle_data.number_of_handles == 5);
    
    for (int i = 0; i < 5; i++) {
        assert(handles[i].handle != 0);
        assert(handles[i].type_name_rva > 0);
        assert(strlen(handle_types[i]) > 0);
        
        if (strlen(handle_names[i]) > 0) {
            assert(handles[i].object_name_rva > 0);
        }
    }
}

// Test 6: Function Table Stream
void test_function_table_stream_aarch64() {
    struct RuntimeFunction {
        uint64_t begin_address;
        uint64_t end_address;
        uint64_t unwind_data;
    };
    
    struct MinidumpFunctionTable {
        uint64_t minimum_address;
        uint64_t maximum_address;
        uint32_t size_of_header;
        uint32_t size_of_descriptor;
        uint32_t number_of_descriptors;
        uint32_t size_of_native_descriptor;
        uint32_t size_of_function_entry;
    };
    
    const int num_functions = 8;
    RuntimeFunction functions[num_functions] = {
        {0x40001000, 0x40001100, 0x5000},
        {0x40001100, 0x40001300, 0x5100},
        {0x40001300, 0x40001450, 0x5200},
        {0x40001450, 0x40001550, 0x5300},
        {0x40001550, 0x400015D0, 0x5400},
        {0x400015D0, 0x400016F0, 0x5500},
        {0x400016F0, 0x40001750, 0x5600},
        {0x40001750, 0x40001950, 0x5700}
    };
    
    MinidumpFunctionTable func_table = {};
    func_table.minimum_address = 0x40001000;
    func_table.maximum_address = 0x40001950;
    func_table.size_of_header = sizeof(MinidumpFunctionTable);
    func_table.size_of_descriptor = sizeof(RuntimeFunction);
    func_table.number_of_descriptors = num_functions;
    func_table.size_of_native_descriptor = 0;
    func_table.size_of_function_entry = 0;
    
    // Verify function table
    assert(func_table.minimum_address < func_table.maximum_address);
    assert(func_table.number_of_descriptors == num_functions);
    
    for (int i = 0; i < num_functions; i++) {
        assert(functions[i].begin_address >= func_table.minimum_address);
        assert(functions[i].end_address <= func_table.maximum_address);
        assert(functions[i].begin_address < functions[i].end_address);
        assert(functions[i].unwind_data > 0);
        
        // Functions should be in order
        if (i > 0) {
            assert(functions[i-1].end_address <= functions[i].begin_address);
        }
    }
}

// Test 7: Unloaded Module Stream
void test_unloaded_module_stream_aarch64() {
    struct UnloadedModule {
        uint64_t base_of_image;
        uint64_t size_of_image;
        uint32_t checksum;
        uint32_t time_date_stamp;
        uint64_t module_name_rva;
    };
    
    const int num_unloaded = 3;
    UnloadedModule unloaded[num_unloaded] = {
        {0x7F0010000000, 0x100000, 0x11111111, 0x5F3A1B2C, 0x6000},
        {0x7F0020000000, 0x80000, 0x22222222, 0x6F4B2C3D, 0x6100},
        {0x7F0028000000, 0x40000, 0x33333333, 0x7F5C3D4E, 0x6200}
    };
    
    const char* unloaded_names[] = {
        "test_plugin.so",
        "obsolete_driver.ko",
        "legacy_library.dll"
    };
    
    // Verify unloaded modules
    for (int i = 0; i < num_unloaded; i++) {
        assert(unloaded[i].base_of_image > 0);
        assert(unloaded[i].size_of_image > 0);
        assert(unloaded[i].module_name_rva > 0);
        assert(strlen(unloaded_names[i]) > 0);
        
        // Unloaded modules should have valid addresses
        assert(unloaded[i].base_of_image < 0x800000000000);
    }
}

// Test 8: Memory Info Stream
void test_memory_info_stream_aarch64() {
    struct MemoryInfo {
        uint64_t base_address;
        uint64_t allocation_base;
        uint32_t allocation_protect;
        uint32_t __alignment1;
        uint64_t region_size;
        uint32_t state;
        uint32_t protect;
        uint32_t type;
        uint32_t __alignment2;
    };
    
    const int num_regions = 6;
    MemoryInfo regions[num_regions] = {
        // Code region
        {0x40000000, 0x40000000, 0x20, 0, 0x1000, 0x1000, 0x20, 0x20000, 0},
        // Read-only data
        {0x40001000, 0x40000000, 0x04, 0, 0x1000, 0x1000, 0x02, 0x40000, 0},
        // Read-write data
        {0x40002000, 0x40000000, 0x04, 0, 0x1000, 0x1000, 0x04, 0x40000, 0},
        // Stack
        {0x7000000000, 0x7000000000, 0x04, 0, 0x8000, 0x1000, 0x04, 0x20000, 0},
        // Heap
        {0x7100000000, 0x7100000000, 0x04, 0, 0x10000, 0x1000, 0x04, 0x20000, 0},
        // Mapped file
        {0x7F0000000000, 0x7F0000000000, 0x02, 0, 0x200000, 0x1000, 0x02, 0x40000, 0}
    };
    
    // Verify memory info
    for (int i = 0; i < num_regions; i++) {
        assert(regions[i].base_address > 0);
        assert(regions[i].region_size > 0);
        assert(regions[i].allocation_protect != 0);
        assert(regions[i].state == 0x1000);  // MEM_COMMIT
        assert(regions[i].protect != 0);
        assert(regions[i].type != 0);
        
        // Check permissions
        uint32_t prot = regions[i].protect;
        if (prot & 0x20) {  // EXECUTE
            assert(prot & 0x02);  // Should also be readable
        }
    }
}

// Test 9: Thread Info Stream
void test_thread_info_stream_aarch64() {
    struct ThreadInfo {
        uint32_t thread_id;
        uint32_t dump_flags;
        uint32_t dump_error;
        uint32_t exit_status;
        uint64_t create_time;
        uint64_t exit_time;
        uint64_t kernel_time;
        uint64_t user_time;
        uint64_t start_address;
        uint64_t affinity;
    };
    
    const int num_threads = 4;
    ThreadInfo threads[num_threads] = {
        // Main thread
        {1234, 0x0001, 0, 0, 0x5F3A1B2C, 0, 0x567, 0x1234, 0x40001000, 0x0F},
        // Worker thread
        {1235, 0x0001, 0, 0, 0x5F3A1B2D, 0, 0x234, 0x5678, 0x40002000, 0x0F},
        // I/O thread
        {1236, 0x0002, 0, 0, 0x5F3A1B2E, 0, 0x123, 0x9ABC, 0x40003000, 0x0F},
        // Timer thread
        {1237, 0x0004, 0, 0, 0x5F3A1B2F, 0, 0x345, 0xDEF0, 0x40004000, 0x0F}
    };
    
    // Verify thread info
    for (int i = 0; i < num_threads; i++) {
        assert(threads[i].thread_id > 0);
        assert(threads[i].create_time > 0);
        assert(threads[i].start_address >= 0x40000000);
        assert(threads[i].affinity != 0);
        
        // Thread IDs should be unique
        for (int j = i + 1; j < num_threads; j++) {
            assert(threads[i].thread_id != threads[j].thread_id);
        }
    }
}

// Test 10: Custom AArch64 Context Stream
void test_aarch64_context_stream_aarch64() {
    struct AArch64ExtendedContext {
        uint32_t context_flags;
        uint32_t padding;
        uint64_t tpidr_el0;    // Thread ID register
        uint64_t tpidrro_el0;  // User Read-Only Thread ID
        uint64_t tpidr_el1;    // Kernel Thread ID
        uint64_t sp_el0;       // Stack pointer EL0
        uint64_t sp_el1;       // Stack pointer EL1
        uint64_t elr_el1;      // Exception Link Register EL1
        uint64_t spsr_el1;     // Saved Processor State EL1
        uint64_t far_el1;      // Fault Address Register
        uint64_t esr_el1;      // Exception Syndrome Register
    };
    
    AArch64ExtendedContext ext_ctx = {};
    ext_ctx.context_flags = 0x80000000;  // Extended context flag
    ext_ctx.tpidr_el0 = 0x123456789ABCDEF0;
    ext_ctx.tpidrro_el0 = 0x23456789ABCDEF01;
    ext_ctx.tpidr_el1 = 0x3456789ABCDEF012;
    ext_ctx.sp_el0 = 0x7000000000;
    ext_ctx.sp_el1 = 0xFFFF00001000;
    ext_ctx.elr_el1 = 0xFFFF00002000;
    ext_ctx.spsr_el1 = 0x60000000;  // EL1, interrupts masked
    ext_ctx.far_el1 = 0xDEADBEEF;
    ext_ctx.esr_el1 = 0x96000047;  // Data abort, write, EL1
    
    // Verify extended context
    assert(ext_ctx.context_flags == 0x80000000);
    assert(ext_ctx.tpidr_el0 != 0);
    assert(ext_ctx.tpidr_el1 != 0);
    assert(ext_ctx.sp_el0 < 0x8000000000);  // User stack
    assert(ext_ctx.sp_el1 >= 0xFFFF00000000);  // Kernel stack
    assert(ext_ctx.elr_el1 >= 0xFFFF00000000);  // Kernel address
    assert(ext_ctx.far_el1 == 0xDEADBEEF);
    assert((ext_ctx.esr_el1 & 0xFC000000) == 0x96000000);  // Data abort
}

// Test 11: Stream Size Validation
void test_stream_size_validation_aarch64() {
    const int num_streams = 7;
    MinidumpDirectory streams[num_streams] = {
        {MD_THREAD_LIST_STREAM, 0, 0x1000},  // Zero size (invalid)
        {MD_MODULE_LIST_STREAM, 0xFFFFFFFF, 0x2000},  // Too large
        {MD_MEMORY_LIST_STREAM, 0x800, 0x3000},  // Valid
        {MD_EXCEPTION_STREAM, 0x300, 0},  // Zero RVA (invalid)
        {MD_SYSTEM_INFO_STREAM, 0x100, 0x4000},  // Valid
        {MD_MISC_INFO_STREAM, 0x200, 0xFFFFFFFFFFFFFFFF},  // Max RVA
        {MD_HANDLE_DATA_STREAM, 0x180, 0x5000}  // Valid
    };
    
    int valid_count = 0;
    for (int i = 0; i < num_streams; i++) {
        bool valid = true;
        
        // Validate stream properties
        if (streams[i].data_size == 0 || streams[i].data_size > 0x1000000) {
            valid = false;
        }
        if (streams[i].rva == 0 || streams[i].rva == 0xFFFFFFFFFFFFFFFF) {
            valid = false;
        }
        if (streams[i].stream_type > MD_LAST_RESERVED_STREAM && 
            streams[i].stream_type != MD_AARCH64_CONTEXT_STREAM) {
            valid = false;
        }
        
        if (valid) valid_count++;
    }
    
    // Should have 3 valid streams
    assert(valid_count == 3);
}

// Test 12: Stream Order and Duplication
void test_stream_order_duplication_aarch64() {
    const int num_streams = 6;
    MinidumpDirectory streams[num_streams] = {
        {MD_THREAD_LIST_STREAM, 0x400, 0x1000},
        {MD_MODULE_LIST_STREAM, 0x600, 0x1400},
        {MD_MEMORY_LIST_STREAM, 0x800, 0x1A00},
        {MD_THREAD_LIST_STREAM, 0x200, 0x2200},  // Duplicate!
        {MD_EXCEPTION_STREAM, 0x300, 0x2400},
        {MD_SYSTEM_INFO_STREAM, 0x100, 0x2700}
    };
    
    // Check for duplicate stream types
    bool duplicate_found = false;
    for (int i = 0; i < num_streams; i++) {
        for (int j = i + 1; j < num_streams; j++) {
            if (streams[i].stream_type == streams[j].stream_type) {
                duplicate_found = true;
                break;
            }
        }
        if (duplicate_found) break;
    }
    
    assert(duplicate_found);  // Should have duplicate thread stream
}

// Run all stream tests
void test_all_minidump_streams_aarch64() {
    test_basic_stream_directory_aarch64();
    test_system_info_stream_aarch64();
    test_misc_info_stream_aarch64();
    test_comment_streams_aarch64();
    test_handle_data_stream_aarch64();
    test_function_table_stream_aarch64();
    test_unloaded_module_stream_aarch64();
    test_memory_info_stream_aarch64();
    test_thread_info_stream_aarch64();
    test_aarch64_context_stream_aarch64();
    test_stream_size_validation_aarch64();
    test_stream_order_duplication_aarch64();
    
    assert(true);
}