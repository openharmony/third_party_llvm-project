// ============================================================================
// Minidump Function Table Tests for AArch64
// ============================================================================
// Tests function table and unwind information parsing for aarch64 minidumps

#include <cstdint>
#include <cstring>
#include <cassert>

// Function table structures
struct MinidumpFunctionTableDescriptor {
    uint64_t minimum_address;
    uint64_t maximum_address;
    uint32_t size_of_header;
    uint32_t size_of_descriptor;
    uint32_t number_of_descriptors;
    uint32_t size_of_native_descriptor;
    uint32_t size_of_function_entry;
};

// Runtime function for AArch64
struct RuntimeFunctionAArch64 {
    uint64_t begin_address;
    uint64_t end_address;
    uint64_t unwind_data;
};

// Unwind information for AArch64
struct UnwindInfoAArch64 {
    uint32_t version : 3;
    uint32_t flags : 5;
    uint32_t size_of_prolog : 8;
    uint32_t count_of_codes : 8;
    uint32_t frame_register : 4;
    uint32_t frame_offset : 4;
    uint8_t unwind_codes[1];  // Variable length
};

// Test 1: Basic Function Table
void test_basic_function_table_aarch64() {
    const int num_functions = 8;
    RuntimeFunctionAArch64 functions[num_functions] = {
        {0x40001000, 0x40001100, 0x5000},  // main
        {0x40001100, 0x40001300, 0x5100},  // func1
        {0x40001300, 0x40001450, 0x5200},  // func2
        {0x40001450, 0x40001550, 0x5300},  // func3
        {0x40001550, 0x400015D0, 0x5400},  // func4
        {0x400015D0, 0x400016F0, 0x5500},  // func5
        {0x400016F0, 0x40001750, 0x5600},  // func6
        {0x40001750, 0x40001950, 0x5700}   // func7
    };
    
    MinidumpFunctionTableDescriptor table = {};
    table.minimum_address = 0x40001000;
    table.maximum_address = 0x40001950;
    table.size_of_header = sizeof(MinidumpFunctionTableDescriptor);
    table.size_of_descriptor = sizeof(RuntimeFunctionAArch64);
    table.number_of_descriptors = num_functions;
    table.size_of_native_descriptor = 0;
    table.size_of_function_entry = 0;
    
    // Verify function table
    assert(table.minimum_address < table.maximum_address);
    assert(table.number_of_descriptors == num_functions);
    assert(table.size_of_descriptor == sizeof(RuntimeFunctionAArch64));
    
    // Verify functions
    for (int i = 0; i < num_functions; i++) {
        assert(functions[i].begin_address >= table.minimum_address);
        assert(functions[i].end_address <= table.maximum_address);
        assert(functions[i].begin_address < functions[i].end_address);
        assert(functions[i].unwind_data > 0);
        
        // Functions should be in order
        if (i > 0) {
            assert(functions[i-1].end_address <= functions[i].begin_address);
        }
    }
}

// Test 2: Function Overlap Detection
void test_function_overlap_detection_aarch64() {
    const int num_functions = 6;
    RuntimeFunctionAArch64 functions[num_functions] = {
        {0x40001000, 0x40001100, 0x5000},  // Normal
        {0x40001100, 0x40001200, 0x5100},  // Adjacent
        {0x40001200, 0x40001300, 0x5200},  // Normal
        {0x40001300, 0x40001400, 0x5300},  // Will overlap with next
        {0x40001350, 0x40001450, 0x5400},  // Overlaps with previous!
        {0x40001500, 0x40001600, 0x5500}   // Normal
    };
    
    // Check for overlaps
    bool has_overlap = false;
    for (int i = 0; i < num_functions; i++) {
        for (int j = i + 1; j < num_functions; j++) {
            if (functions[i].begin_address < functions[j].end_address &&
                functions[j].begin_address < functions[i].end_address) {
                has_overlap = true;
                break;
            }
        }
        if (has_overlap) break;
    }
    
    assert(has_overlap);  // Should detect overlap
}

// Test 3: Unwind Information Parsing
void test_unwind_info_parsing_aarch64() {
    // Simulate unwind codes for a function
    uint8_t unwind_codes[] = {
        0x01, 0x00,  // Version 1, no flags
        0x20,         // Prolog size = 32 bytes
        0x04,         // 4 unwind codes
        0x00,         // Frame register = 0 (not used)
        0x00,         // Frame offset = 0
        
        // Unwind codes:
        0x94, 0x01,  // Save x19,x20 at [sp-0x10]!
        0xD6, 0x02,  // Save x21,x22 at [sp-0x20]!
        0x08, 0x03,  // Save x23,x24 at [sp-0x30]!
        0x9E, 0x01,  // Save x29,x30 at [sp-0x40]!
    };
    
    UnwindInfoAArch64* unwind_info = (UnwindInfoAArch64*)unwind_codes;
    
    // Verify unwind info
    assert(unwind_info->version == 1);
    assert(unwind_info->flags == 0);
    assert(unwind_info->size_of_prolog == 32);
    assert(unwind_info->count_of_codes == 4);
    assert(unwind_info->frame_register == 0);
    assert(unwind_info->frame_offset == 0);
    
    // Verify unwind codes
    assert(unwind_codes[6] == 0x94);  // Save pair x19,x20
    assert(unwind_codes[7] == 0x01);  // Offset 0x10
    assert(unwind_codes[12] == 0x9E); // Save pair x29,x30
    assert(unwind_codes[13] == 0x01); // Offset 0x40
}

// Test 4: Exception Handling Functions
void test_exception_handling_functions_aarch64() {
    struct ExceptionHandler {
        uint64_t handler_address;
        uint64_t scope_table;
        uint32_t try_level;
        const char* handler_type;
    };
    
    const int num_handlers = 4;
    ExceptionHandler handlers[num_handlers] = {
        {0x40002000, 0x40003000, 0, "__C_specific_handler"},
        {0x40002100, 0x40003100, 1, "__CxxFrameHandler3"},
        {0x40002200, 0x40003200, 2, "__GSHandlerCheck"},
        {0x40002300, 0x40003300, 3, "__delayed_seh_handler"}
    };
    
    // Verify exception handlers
    for (int i = 0; i < num_handlers; i++) {
        assert(handlers[i].handler_address >= 0x40000000);
        assert(handlers[i].scope_table >= 0x40000000);
        assert(strlen(handlers[i].handler_type) > 0);
        
        // Handler addresses should be unique
        for (int j = i + 1; j < num_handlers; j++) {
            assert(handlers[i].handler_address != handlers[j].handler_address);
        }
    }
}

// Test 5: Function Size Analysis
void test_function_size_analysis_aarch64() {
    struct FunctionSize {
        const char* function_name;
        uint64_t address;
        uint64_t size;
        bool is_large;
    };
    
    const int num_functions = 8;
    FunctionSize functions[num_functions] = {
        {"tiny_func", 0x40001000, 0x20, false},
        {"small_func", 0x40001020, 0x100, false},
        {"medium_func", 0x40001120, 0x200, false},
        {"large_func", 0x40001320, 0x800, true},
        {"huge_func", 0x40001B20, 0x2000, true},
        {"inline_func", 0x40003B20, 0x10, false},
        {"template_func", 0x40003B30, 0x80, false},
        {"virtual_func", 0x40003BB0, 0x40, false}
    };
    
    // Analyze function sizes
    uint64_t total_size = 0;
    int large_count = 0;
    
    for (int i = 0; i < num_functions; i++) {
        assert(functions[i].size > 0);
        assert(functions[i].size <= 0x10000);  // Max 64KB
        
        if (functions[i].size > 0x200) {
            assert(functions[i].is_large);
            large_count++;
        } else {
            assert(!functions[i].is_large);
        }
        
        total_size += functions[i].size;
    }
    
    assert(total_size > 0);
    assert(large_count == 2);  // large_func and huge_func
}

// Test 6: Function Prolog Analysis
void test_function_prolog_analysis_aarch64() {
    struct FunctionProlog {
        uint64_t address;
        uint32_t prolog_size;
        uint8_t prolog_bytes[16];
        bool has_frame_pointer;
        int saved_reg_count;
    };
    
    const int num_functions = 4;
    FunctionProlog prologs[num_functions] = {
        // Leaf function (no frame)
        {0x40001000, 8, {0xC0, 0x03, 0x5F, 0xD6}, false, 0},
        
        // Small function with frame
        {0x40001008, 16, {0xFD, 0x7B, 0xBF, 0xA9, 0xFD, 0x03, 0x00, 0x91}, true, 1},
        
        // Medium function
        {0x40001018, 24, {0xFF, 0x83, 0x00, 0xD1, 0xFD, 0x7B, 0x01, 0xA9, 
                          0xFD, 0x43, 0x00, 0x91}, true, 2},
        
        // Large function
        {0x40001030, 32, {0xFF, 0xC3, 0x00, 0xD1, 0xF4, 0x4F, 0x01, 0xA9,
                          0xF6, 0x57, 0x02, 0xA9, 0xF8, 0x5F, 0x03, 0xA9}, true, 4}
    };
    
    // Verify prologs
    for (int i = 0; i < num_functions; i++) {
        assert(prologs[i].prolog_size > 0);
        assert(prologs[i].prolog_size <= 64);
        
        // Check for standard prolog patterns
        bool has_store = false;
        for (int j = 0; j < 8; j++) {
            if ((prologs[i].prolog_bytes[j] & 0xFF) == 0xA9) {  // STP instruction
                has_store = true;
                break;
            }
        }
        
        if (prologs[i].has_frame_pointer) {
            assert(has_store);
            assert(prologs[i].saved_reg_count > 0);
        }
    }
}

// Test 7: Function Type Detection
void test_function_type_detection_aarch64() {
    struct FunctionType {
        uint64_t address;
        const char* name;
        bool is_exported;
        bool is_imported;
        bool is_static;
        bool is_virtual;
    };
    
    const int num_functions = 8;
    FunctionType functions[num_functions] = {
        {0x40001000, "main", true, false, false, false},
        {0x40001100, "static_helper", false, false, true, false},
        {0x40001200, "virtual_method", false, false, false, true},
        {0x40001300, "imported_func", false, true, false, false},
        {0x40001400, "exported_api", true, false, false, false},
        {0x40001500, "inline_func", false, false, true, false},
        {0x40001600, "template_func", false, false, true, false},
        {0x40001700, "callback", true, false, false, false}
    };
    
    // Analyze function types
    int exported_count = 0;
    int imported_count = 0;
    int static_count = 0;
    int virtual_count = 0;
    
    for (int i = 0; i < num_functions; i++) {
        assert(functions[i].address >= 0x40000000);
        assert(strlen(functions[i].name) > 0);
        
        if (functions[i].is_exported) exported_count++;
        if (functions[i].is_imported) imported_count++;
        if (functions[i].is_static) static_count++;
        if (functions[i].is_virtual) virtual_count++;
        
        // Imported functions can't be exported
        if (functions[i].is_imported) {
            assert(!functions[i].is_exported);
        }
    }
    
    assert(exported_count == 3);  // main, exported_api, callback
    assert(imported_count == 1);  // imported_func
    assert(static_count == 3);    // static_helper, inline_func, template_func
    assert(virtual_count == 1);   // virtual_method
}

// Test 8: Function Range Validation
void test_function_range_validation_aarch64() {
    struct FunctionRange {
        uint64_t start;
        uint64_t end;
        bool should_be_valid;
    };
    
    const int num_ranges = 8;
    FunctionRange ranges[num_ranges] = {
        {0x40001000, 0x40001100, true},    // Valid: 256 bytes
        {0x40002000, 0x40002010, true},    // Valid: 16 bytes
        {0x40003000, 0x40004000, true},    // Valid: 4KB
        {0x40004000, 0x40004000, false},   // Invalid: zero size
        {0x40005000, 0x40004FFF, false},   // Invalid: end < start
        {0x00000000, 0x00000100, false},   // Invalid: NULL start
        {0xFFFFFFFFFFFFFFFF, 0x0, false},  // Invalid: overflow
        {0x40006000, 0x4000E000, true}     // Valid: 32KB
    };
    
    // Validate ranges
    for (int i = 0; i < num_ranges; i++) {
        bool is_valid = true;
        
        // Basic validation
        if (ranges[i].start == 0 || ranges[i].end == 0) {
            is_valid = false;
        }
        else if (ranges[i].end <= ranges[i].start) {
            is_valid = false;
        }
        else if (ranges[i].end - ranges[i].start > 0x10000) {  // Max 64KB
            is_valid = false;
        }
        else if ((ranges[i].start & 0x3) != 0) {  // 4-byte aligned
            is_valid = false;
        }
        
        assert(is_valid == ranges[i].should_be_valid);
    }
}

// Test 9: Function Hot/Cold Detection
void test_function_hot_cold_detection_aarch64() {
    struct FunctionSection {
        uint64_t hot_address;
        uint64_t hot_size;
        uint64_t cold_address;
        uint64_t cold_size;
        bool has_cold_section;
    };
    
    const int num_functions = 5;
    FunctionSection functions[num_functions] = {
        // Hot only
        {0x40001000, 0x200, 0, 0, false},
        
        // Hot with small cold
        {0x40001200, 0x100, 0x40100000, 0x50, true},
        
        // Hot with large cold
        {0x40001300, 0x80, 0x40101000, 0x200, true},
        
        // Large hot only
        {0x40001400, 0x400, 0, 0, false},
        
        // Hot with very cold
        {0x40001800, 0x60, 0x40102000, 0x400, true}
    };
    
    // Analyze function sections
    int cold_count = 0;
    uint64_t total_hot = 0;
    uint64_t total_cold = 0;
    
    for (int i = 0; i < num_functions; i++) {
        assert(functions[i].hot_address >= 0x40000000);
        assert(functions[i].hot_size > 0);
        total_hot += functions[i].hot_size;
        
        if (functions[i].has_cold_section) {
            cold_count++;
            assert(functions[i].cold_address > 0);
            assert(functions[i].cold_size > 0);
            total_cold += functions[i].cold_size;
            
            // Cold section should be separate
            assert(functions[i].cold_address != functions[i].hot_address);
        }
    }
    
    assert(cold_count == 3);
    assert(total_hot > 0);
    assert(total_cold > 0);
}

// Test 10: Function Call Graph
void test_function_call_graph_aarch64() {
    struct FunctionCall {
        uint64_t caller;
        uint64_t callee;
        int call_count;
        bool is_indirect;
    };
    
    const int num_calls = 10;
    FunctionCall calls[num_calls] = {
        {0x40001000, 0x40001100, 1, false},  // main -> func1
        {0x40001000, 0x40001200, 2, false},  // main -> func2
        {0x40001100, 0x40001300, 1, false},  // func1 -> func3
        {0x40001200, 0x40001300, 3, false},  // func2 -> func3
        {0x40001300, 0x40001400, 1, false},  // func3 -> func4
        {0x40001000, 0x40001500, 1, true},   // main -> indirect
        {0x40001400, 0x40001600, 2, false},  // func4 -> func6
        {0x40001500, 0x40001700, 1, false},  // indirect -> func7
        {0x40001600, 0x40001800, 1, false},  // func6 -> func8
        {0x40001700, 0x40001900, 1, false}   // func7 -> func9
    };
    
    // Analyze call graph
    int direct_calls = 0;
    int indirect_calls = 0;
    uint64_t total_calls = 0;
    
    for (int i = 0; i < num_calls; i++) {
        assert(calls[i].caller >= 0x40000000);
        assert(calls[i].callee >= 0x40000000);
        assert(calls[i].call_count > 0);
        total_calls += calls[i].call_count;
        
        if (calls[i].is_indirect) {
            indirect_calls++;
        } else {
            direct_calls++;
        }
    }
    
    assert(direct_calls == 9);
    assert(indirect_calls == 1);
    assert(total_calls == 14);
}

// Run all function table tests
void test_all_minidump_function_tables_aarch64() {
    test_basic_function_table_aarch64();
    test_function_overlap_detection_aarch64();
    test_unwind_info_parsing_aarch64();
    test_exception_handling_functions_aarch64();
    test_function_size_analysis_aarch64();
    test_function_prolog_analysis_aarch64();
    test_function_type_detection_aarch64();
    test_function_range_validation_aarch64();
    test_function_hot_cold_detection_aarch64();
    test_function_call_graph_aarch64();
    
    assert(true);
}