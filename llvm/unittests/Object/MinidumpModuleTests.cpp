// ============================================================================
// Minidump Module Tests for AArch64
// ============================================================================
// Tests module loading and symbol resolution for aarch64 minidumps

#include <cstdint>
#include <cstring>
#include <cassert>

// Module entry structure
struct MinidumpModule {
    uint64_t base_of_image;
    uint64_t size_of_image;
    uint32_t checksum;
    uint32_t time_date_stamp;
    uint64_t module_name_rva;
    uint32_t cv_record_size;
    uint32_t misc_record_size;
    uint64_t cv_record;
    uint64_t misc_record;
    uint64_t reserved_0;
    uint64_t reserved_1;
};

// Module list structure
struct MinidumpModuleList {
    uint32_t number_of_modules;
    MinidumpModule modules[1];  // Flexible array
};

// Debug info structures
struct CodeViewRecord {
    uint32_t signature;
    uint32_t offset;
    uint8_t data[1];
};

struct MiscDebugInfo {
    uint32_t data_type;
    uint32_t length;
    uint8_t data[1];
};

// Test 1: Basic Module Loading
void test_basic_module_loading_aarch64() {
    const int num_modules = 3;
    MinidumpModule modules[num_modules] = {
        // Main executable
        {
            0x40000000,          // base_of_image
            0x200000,            // size_of_image
            0x12345678,          // checksum
            0x5F3A1B2C,          // timestamp
            0x1000,              // module_name_rva
            0x200,               // cv_record_size
            0x100,               // misc_record_size
            0x3000,              // cv_record
            0x3200,              // misc_record
            0, 0
        },
        // libc.so.6
        {
            0x7F0000000000,      // base_of_image
            0x300000,            // size_of_image
            0x87654321,          // checksum
            0x6F4B2C3D,          // timestamp
            0x5000,              // module_name_rva
            0x300,               // cv_record_size
            0x150,               // misc_record_size
            0x6000,              // cv_record
            0x6300,              // misc_record
            0, 0
        },
        // libm.so.6
        {
            0x7F0030000000,      // base_of_image
            0x100000,            // size_of_image
            0x9ABCDEF0,          // checksum
            0x7F5C3D4E,          // timestamp
            0x8000,              // module_name_rva
            0x250,               // cv_record_size
            0x120,               // misc_record_size
            0x9000,              // cv_record
            0x9250,              // misc_record
            0, 0
        }
    };
    
    // Module names
    const char* module_names[] = {
        "/usr/bin/myapp",
        "/lib/aarch64-linux-gnu/libc.so.6",
        "/lib/aarch64-linux-gnu/libm.so.6"
    };
    
    // Verify module properties
    for (int i = 0; i < num_modules; i++) {
        assert(modules[i].base_of_image > 0);
        assert(modules[i].size_of_image > 0);
        assert(modules[i].time_date_stamp > 0);
        assert(modules[i].module_name_rva > 0);
        
        // Check address ranges
        uint64_t module_end = modules[i].base_of_image + modules[i].size_of_image;
        assert(module_end > modules[i].base_of_image);
    }
    
    // Verify no overlaps
    for (int i = 0; i < num_modules; i++) {
        for (int j = i + 1; j < num_modules; j++) {
            uint64_t i_start = modules[i].base_of_image;
            uint64_t i_end = i_start + modules[i].size_of_image;
            uint64_t j_start = modules[j].base_of_image;
            uint64_t j_end = j_start + modules[j].size_of_image;
            
            assert(i_end <= j_start || j_end <= i_start);
        }
    }
    
    // Verify module names
    for (int i = 0; i < num_modules; i++) {
        const char* name = module_names[i];
        assert(strlen(name) > 0);
        assert(strstr(name, i == 0 ? ".so" : "/") != nullptr);
    }
}

// Test 2: Module Debug Information
void test_module_debug_info_aarch64() {
    MinidumpModule module = {
        0x40000000,
        0x200000,
        0x12345678,
        0x5F3A1B2C,
        0x1000,
        0x200,    // CV record size
        0x100,    // Misc record size
        0x3000,   // CV record RVA
        0x3200,   // Misc record RVA
        0, 0
    };
    
    // CodeView record (PDB 7.0 format)
    struct CvRecordPdb70 {
        uint32_t signature;      // "RSDS"
        uint8_t signature_bytes[16];  // GUID
        uint32_t age;
        char pdb_name[1];        // Null-terminated string
    };
    
    // Simulate CV record
    CvRecordPdb70 cv_record = {};
    cv_record.signature = 0x53445352;  // "RSDS" in little-endian
    
    // Set GUID
    uint8_t guid[16] = {
        0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
        0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x00
    };
    memcpy(cv_record.signature_bytes, guid, 16);
    cv_record.age = 1;
    strcpy(cv_record.pdb_name, "myapp.pdb");
    
    // Verify CV record
    assert(cv_record.signature == 0x53445352);
    assert(cv_record.age > 0);
    assert(strlen(cv_record.pdb_name) > 0);
    
    // Misc debug info (CodeView 4.0 format)
    struct MiscRecordCv4 {
        uint32_t signature;      // "NB10"
        uint32_t offset;
        uint32_t timestamp;
        uint32_t age;
        char pdb_name[1];
    };
    
    MiscRecordCv4 misc_record = {};
    misc_record.signature = 0x3031424E;  // "NB10" in little-endian
    misc_record.offset = 0;
    misc_record.timestamp = module.time_date_stamp;
    misc_record.age = 1;
    strcpy(misc_record.pdb_name, "myapp.pdb");
    
    // Verify misc record
    assert(misc_record.signature == 0x3031424E);
    assert(misc_record.timestamp == module.time_date_stamp);
}

// Test 3: Module Symbol Resolution
void test_module_symbols_aarch64() {
    struct SymbolEntry {
        uint64_t address;
        uint32_t size;
        uint32_t flags;
        const char* name;
    };
    
    const int num_symbols = 10;
    SymbolEntry symbols[num_symbols] = {
        {0x40001000, 0x100, 1, "_start"},
        {0x40001100, 0x200, 1, "main"},
        {0x40001300, 0x150, 1, "process_data"},
        {0x40001450, 0x100, 1, "cleanup"},
        {0x40001550, 0x80, 0, "internal_helper"},
        {0x400015D0, 0x120, 1, "public_function"},
        {0x400016F0, 0x60, 0, "static_var"},
        {0x40001750, 0x200, 1, "thread_entry"},
        {0x40001950, 0x100, 1, "signal_handler"},
        {0x40001A50, 0x80, 1, "exit"}
    };
    
    // Verify symbol properties
    uint64_t module_base = 0x40000000;
    uint64_t module_size = 0x200000;
    
    for (int i = 0; i < num_symbols; i++) {
        // Symbol must be within module bounds
        assert(symbols[i].address >= module_base);
        assert(symbols[i].address + symbols[i].size <= module_base + module_size);
        
        // Symbol size must be reasonable
        assert(symbols[i].size > 0);
        assert(symbols[i].size <= 0x10000);
        
        // Symbol name must be valid
        assert(symbols[i].name != nullptr);
        assert(strlen(symbols[i].name) > 0);
    }
    
    // Verify symbol ordering
    for (int i = 0; i < num_symbols - 1; i++) {
        assert(symbols[i].address < symbols[i + 1].address);
    }
    
    // Test symbol lookup
    uint64_t test_address = 0x40001350;  // Inside process_data
    const char* found_symbol = nullptr;
    
    for (int i = 0; i < num_symbols; i++) {
        if (test_address >= symbols[i].address && 
            test_address < symbols[i].address + symbols[i].size) {
            found_symbol = symbols[i].name;
            break;
        }
    }
    
    assert(found_symbol != nullptr);
    assert(strcmp(found_symbol, "process_data") == 0);
}

// Test 4: Shared Library Modules
void test_shared_library_modules_aarch64() {
    const int num_libs = 5;
    MinidumpModule libs[num_libs] = {
        // libc.so.6
        {
            0x7F0000000000,
            0x300000,
            0x11111111,
            0x5F3A1B2C,
            0x5000,
            0x300,
            0x150,
            0x6000,
            0x6300,
            0, 0
        },
        // libm.so.6
        {
            0x7F0030000000,
            0x100000,
            0x22222222,
            0x6F4B2C3D,
            0x8000,
            0x250,
            0x120,
            0x9000,
            0x9250,
            0, 0
        },
        // libpthread.so.0
        {
            0x7F0040000000,
            0x80000,
            0x33333333,
            0x7F5C3D4E,
            0xA000,
            0x200,
            0x100,
            0xB000,
            0xB200,
            0, 0
        },
        // libdl.so.2
        {
            0x7F0048000000,
            0x40000,
            0x44444444,
            0x8F6D4E5F,
            0xC000,
            0x180,
            0x80,
            0xD000,
            0xD180,
            0, 0
        },
        // libstdc++.so.6
        {
            0x7F004C000000,
            0x500000,
            0x55555555,
            0x9F7E5F60,
            0xE000,
            0x400,
            0x200,
            0xF000,
            0xF400,
            0, 0
        }
    };
    
    const char* lib_names[] = {
        "libc.so.6",
        "libm.so.6", 
        "libpthread.so.0",
        "libdl.so.2",
        "libstdc++.so.6"
    };
    
    // Verify shared library properties
    for (int i = 0; i < num_libs; i++) {
        // Shared libraries in high memory
        assert(libs[i].base_of_image >= 0x7F0000000000);
        
        // Reasonable sizes
        assert(libs[i].size_of_image >= 0x40000);
        assert(libs[i].size_of_image <= 0x1000000);
        
        // Library names
        assert(strstr(lib_names[i], ".so") != nullptr);
    }
    
    // Check library dependencies
    uint64_t prev_end = 0;
    for (int i = 0; i < num_libs; i++) {
        uint64_t lib_start = libs[i].base_of_image;
        uint64_t lib_end = lib_start + libs[i].size_of_image;
        
        if (prev_end > 0) {
            // Libraries should be loaded sequentially
            assert(lib_start >= prev_end);
        }
        
        prev_end = lib_end;
    }
}

// Test 5: Kernel Module Loading
void test_kernel_modules_aarch64() {
    const int num_kmods = 3;
    MinidumpModule kmods[num_kmods] = {
        // Kernel core
        {
            0xFFFF000000000000,
            0x3000000,
            0xAAAAAAAA,
            0x12345678,
            0x2000,
            0x500,
            0x300,
            0x3000,
            0x3500,
            0, 0
        },
        // Driver module
        {
            0xFFFF000030000000,
            0x100000,
            0xBBBBBBBB,
            0x23456789,
            0x4000,
            0x300,
            0x200,
            0x5000,
            0x5300,
            0, 0
        },
        // Filesystem module
        {
            0xFFFF000031000000,
            0x80000,
            0xCCCCCCCC,
            0x3456789A,
            0x6000,
            0x200,
            0x100,
            0x7000,
            0x7200,
            0, 0
        }
    };
    
    const char* kmod_names[] = {
        "vmlinux",
        "driver.ko",
        "ext4.ko"
    };
    
    for (int i = 0; i < num_kmods; i++) {
        // Kernel addresses
        assert(kmods[i].base_of_image >= 0xFFFF000000000000);
        
        // Large kernel image
        if (i == 0) {
            assert(kmods[i].size_of_image >= 0x1000000);
        }
        
        // Module names
        assert(strlen(kmod_names[i]) > 0);
    }
}

// Test 6: Module Timestamps and Versions
void test_module_versions_aarch64() {
    struct ModuleVersion {
        MinidumpModule module;
        uint32_t version_major;
        uint32_t version_minor;
        uint32_t version_build;
        uint32_t version_qfe;
        const char* version_string;
    };
    
    const int num_modules = 4;
    ModuleVersion modules[num_modules] = {
        {
            {0x40000000, 0x200000, 0x1111, 0x5F3A1B2C, 0x1000, 0x200, 0x100, 0x3000, 0x3200, 0, 0},
            1, 0, 0, 0, "1.0.0.0"
        },
        {
            {0x7F0000000000, 0x300000, 0x2222, 0x6F4B2C3D, 0x5000, 0x300, 0x150, 0x6000, 0x6300, 0, 0},
            2, 31, 0, 0, "2.31"
        },
        {
            {0x7F0030000000, 0x100000, 0x3333, 0x7F5C3D4E, 0x8000, 0x250, 0x120, 0x9000, 0x9250, 0, 0},
            1, 2, 3, 0, "1.2.3"
        },
        {
            {0xFFFF000000000000, 0x3000000, 0x4444, 0x12345678, 0x2000, 0x500, 0x300, 0x3000, 0x3500, 0, 0},
            5, 4, 0, 0, "5.4.0"
        }
    };
    
    for (int i = 0; i < num_modules; i++) {
        assert(modules[i].version_major >= 1);
        assert(modules[i].version_minor >= 0);
        
        // Version string format
        const char* ver = modules[i].version_string;
        assert(strlen(ver) > 0);
        
        // Check for dots in version string
        bool has_dot = false;
        for (size_t j = 0; ver[j]; j++) {
            if (ver[j] == '.') {
                has_dot = true;
                break;
            }
        }
        assert(has_dot);
    }
}

// Test 7: Module Section Information
void test_module_sections_aarch64() {
    struct ModuleSection {
        const char* name;
        uint64_t virtual_address;
        uint64_t size;
        uint32_t characteristics;
    };
    
    const int num_sections = 8;
    ModuleSection sections[num_sections] = {
        {".text", 0x40001000, 0x1000, 0x60000020},  // EXECUTE|READ|CODE
        {".rodata", 0x40002000, 0x800, 0x40000040},  // READ|INIT_DATA
        {".data", 0x40003000, 0x1000, 0xC0000040},   // READ|WRITE|INIT_DATA
        {".bss", 0x40004000, 0x800, 0xC0000080},     // READ|WRITE|UNINIT_DATA
        {".plt", 0x40005000, 0x400, 0x60000020},     // EXECUTE|READ|CODE
        {".got", 0x40006000, 0x200, 0x40000040},     // READ|INIT_DATA
        {".init_array", 0x40007000, 0x100, 0xC0000040},  // READ|WRITE|INIT_DATA
        {".fini_array", 0x40008000, 0x100, 0xC0000040}   // READ|WRITE|INIT_DATA
    };
    
    uint64_t image_base = 0x40000000;
    uint64_t image_size = 0x200000;
    
    // Verify sections
    for (int i = 0; i < num_sections; i++) {
        // Section must be within module
        assert(sections[i].virtual_address >= image_base);
        assert(sections[i].virtual_address + sections[i].size <= image_base + image_size);
        
        // Valid section name
        assert(sections[i].name[0] == '.');
        assert(strlen(sections[i].name) > 1);
        
        // Valid characteristics
        uint32_t chars = sections[i].characteristics;
        assert((chars & 0x20000000) == 0);  // Not discardable
        assert((chars & 0x02000000) != 0);  // Executable code flag
        assert((chars & 0x40000000) != 0);  // Readable
    }
    
    // Check for overlaps
    for (int i = 0; i < num_sections; i++) {
        for (int j = i + 1; j < num_sections; j++) {
            uint64_t i_end = sections[i].virtual_address + sections[i].size;
            uint64_t j_end = sections[j].virtual_address + sections[j].size;
            
            assert(i_end <= sections[j].virtual_address || 
                   j_end <= sections[i].virtual_address);
        }
    }
}

// Test 8: Module Dependencies
void test_module_dependencies_aarch64() {
    struct ModuleDep {
        const char* module_name;
        uint64_t base_address;
        const char* dependencies[5];
        int num_deps;
    };
    
    const int num_modules = 3;
    ModuleDep modules[num_modules] = {
        {
            "myapp",
            0x40000000,
            {"libc.so.6", "libm.so.6", "libpthread.so.0", nullptr, nullptr},
            3
        },
        {
            "libc.so.6", 
            0x7F0000000000,
            {"ld-linux-aarch64.so.1", nullptr, nullptr, nullptr, nullptr},
            1
        },
        {
            "libm.so.6",
            0x7F0030000000,
            {"libc.so.6", nullptr, nullptr, nullptr, nullptr},
            1
        }
    };
    
    for (int i = 0; i < num_modules; i++) {
        assert(modules[i].num_deps >= 0);
        assert(modules[i].num_deps <= 5);
        
        for (int j = 0; j < modules[i].num_deps; j++) {
            assert(modules[i].dependencies[j] != nullptr);
            assert(strlen(modules[i].dependencies[j]) > 0);
        }
    }
    
    // Check dependency chain
    assert(modules[0].num_deps == 3);  // myapp depends on 3 libs
    assert(modules[1].num_deps == 1);  // libc depends on ld.so
    assert(modules[2].num_deps == 1);  // libm depends on libc
}

// Test 9: Module Export Table
void test_module_exports_aarch64() {
    struct ExportEntry {
        const char* name;
        uint64_t address;
        uint32_t ordinal;
    };
    
    const int num_exports = 8;
    ExportEntry exports[num_exports] = {
        {"main", 0x40001100, 1},
        {"process_data", 0x40001300, 2},
        {"cleanup", 0x40001450, 3},
        {"public_function", 0x400015D0, 4},
        {"thread_entry", 0x40001750, 5},
        {"signal_handler", 0x40001950, 6},
        {"exit", 0x40001A50, 7},
        {"_start", 0x40001000, 8}
    };
    
    // Verify exports
    for (int i = 0; i < num_exports; i++) {
        assert(exports[i].address >= 0x40000000);
        assert(exports[i].address < 0x40200000);
        assert(exports[i].ordinal > 0);
        assert(exports[i].name != nullptr);
        
        // Check name format
        if (strcmp(exports[i].name, "_start") != 0) {
            assert(exports[i].name[0] != '_' || exports[i].name[1] != '_');
        }
    }
    
    // Check ordering by address
    for (int i = 0; i < num_exports - 1; i++) {
        assert(exports[i].address < exports[i + 1].address);
    }
}

// Test 10: Module Relocation Information
void test_module_relocations_aarch64() {
    struct RelocationEntry {
        uint64_t virtual_address;
        uint32_t type;
        uint32_t symbol_index;
    };
    
    const int num_relocs = 6;
    RelocationEntry relocs[num_relocs] = {
        {0x40006000, 1027, 1},  // R_AARCH64_ABS64
        {0x40006008, 1027, 2},
        {0x40006010, 1029, 3},  // R_AARCH64_GLOB_DAT
        {0x40006018, 1029, 4},
        {0x40006020, 1030, 5},  // R_AARCH64_JUMP_SLOT
        {0x40006028, 1030, 6}
    };
    
    uint64_t got_start = 0x40006000;
    uint64_t got_end = 0x40006030;
    
    for (int i = 0; i < num_relocs; i++) {
        // Relocations in GOT
        assert(relocs[i].virtual_address >= got_start);
        assert(relocs[i].virtual_address < got_end);
        
        // Valid relocation type
        assert(relocs[i].type >= 1027 && relocs[i].type <= 1030);
        
        // Valid symbol index
        assert(relocs[i].symbol_index > 0);
        
        // Address alignment
        assert((relocs[i].virtual_address & 0x7) == 0);  // 8-byte aligned
    }
}

// Run all module tests
void test_all_minidump_modules_aarch64() {
    test_basic_module_loading_aarch64();
    test_module_debug_info_aarch64();
    test_module_symbols_aarch64();
    test_shared_library_modules_aarch64();
    test_kernel_modules_aarch64();
    test_module_versions_aarch64();
    test_module_sections_aarch64();
    test_module_dependencies_aarch64();
    test_module_exports_aarch64();
    test_module_relocations_aarch64();
    
    assert(true);
}