// ============================================================================
// Minidump Stack Tests for AArch64
// ============================================================================
// Tests stack unwinding and analysis for aarch64 minidumps

#include <cstdint>
#include <cstring>
#include <cassert>

// Stack frame structure for AArch64
struct StackFrameAArch64 {
    uint64_t frame_pointer;
    uint64_t return_address;
    uint64_t saved_registers[8];  // X19-X26 (callee-saved)
    uint64_t stack_arguments[4];  // Stack arguments
};

// Stack unwinding context
struct UnwindContext {
    uint64_t frame_pointer;
    uint64_t stack_pointer;
    uint64_t program_counter;
    uint64_t link_register;
    bool valid;
};

// Test 1: Basic Stack Unwinding
void test_basic_stack_unwinding_aarch64() {
    const int num_frames = 8;
    StackFrameAArch64 frames[num_frames] = {};
    
    // Initialize frame chain
    uint64_t current_fp = 0x7000FFF0;
    uint64_t stack_base = 0x70010000;
    
    for (int i = 0; i < num_frames; i++) {
        frames[i].frame_pointer = current_fp;
        frames[i].return_address = 0x40001000 + i * 0x100;
        
        // Set saved registers
        for (int r = 0; r < 8; r++) {
            frames[i].saved_registers[r] = 0x1000 + i * 0x100 + r;
        }
        
        // Set stack arguments
        for (int a = 0; a < 4; a++) {
            frames[i].stack_arguments[a] = 0x2000 + i * 0x100 + a;
        }
        
        // Calculate next frame pointer (stack grows down)
        current_fp -= 0x30;  // Standard frame size
    }
    
    // Simulate unwinding
    UnwindContext context = {};
    context.frame_pointer = frames[0].frame_pointer;
    context.stack_pointer = stack_base;
    context.program_counter = frames[0].return_address;
    context.link_register = 0x40001000;
    context.valid = true;
    
    int frames_unwound = 0;
    while (context.valid && frames_unwound < num_frames) {
        // Verify current frame
        assert(context.frame_pointer == frames[frames_unwound].frame_pointer);
        assert(context.program_counter == frames[frames_unwound].return_address);
        
        // Move to next frame
        if (frames_unwound < num_frames - 1) {
            context.frame_pointer = frames[frames_unwound + 1].frame_pointer;
            context.program_counter = frames[frames_unwound + 1].return_address;
        } else {
            context.valid = false;
        }
        
        frames_unwound++;
    }
    
    assert(frames_unwound == num_frames);
}

// Test 2: Stack Memory Layout
void test_stack_memory_layout_aarch64() {
    const uint64_t stack_top = 0x70010000;
    const uint64_t stack_bottom = 0x70000000;
    const uint64_t stack_size = stack_top - stack_bottom;  // 64KB
    
    // Stack regions
    struct StackRegion {
        uint64_t address;
        uint64_t size;
        const char* purpose;
    };
    
    const int num_regions = 6;
    StackRegion regions[num_regions] = {
        {stack_top - 0x1000, 0x1000, "Guard page"},
        {stack_top - 0x2000, 0x1000, "Current frame"},
        {stack_top - 0x3000, 0x1000, "Local variables"},
        {stack_top - 0x4000, 0x1000, "Saved registers"},
        {stack_top - 0x5000, 0x1000, "Arguments"},
        {stack_bottom, 0x1000, "Stack base"}
    };
    
    // Verify stack layout
    for (int i = 0; i < num_regions; i++) {
        assert(regions[i].address >= stack_bottom);
        assert(regions[i].address + regions[i].size <= stack_top);
        assert(strlen(regions[i].purpose) > 0);
        
        // Regions should not overlap
        for (int j = i + 1; j < num_regions; j++) {
            uint64_t i_end = regions[i].address + regions[i].size;
            uint64_t j_end = regions[j].address + regions[j].size;
            
            assert(i_end <= regions[j].address || j_end <= regions[i].address);
        }
    }
    
    // Verify stack grows downward
    for (int i = 0; i < num_regions - 1; i++) {
        assert(regions[i].address > regions[i + 1].address);
    }
}

// Test 3: Stack Frame Sizes
void test_stack_frame_sizes_aarch64() {
    struct FunctionFrame {
        const char* function_name;
        uint64_t frame_size;
        uint64_t local_var_size;
        uint64_t saved_regs_size;
        uint64_t padding_size;
    };
    
    const int num_functions = 6;
    FunctionFrame functions[num_functions] = {
        {"small_function", 0x20, 0x10, 0x8, 0x8},
        {"medium_function", 0x40, 0x20, 0x10, 0x10},
        {"large_function", 0x80, 0x40, 0x20, 0x20},
        {"recursive_func", 0x30, 0x18, 0x8, 0x10},
        {"leaf_function", 0x10, 0x8, 0x0, 0x8},
        {"interrupt_handler", 0x100, 0x80, 0x40, 0x40}
    };
    
    // Verify frame sizes
    for (int i = 0; i < num_functions; i++) {
        assert(functions[i].frame_size > 0);
        assert(functions[i].local_var_size < functions[i].frame_size);
        assert(functions[i].saved_regs_size <= functions[i].frame_size);
        assert(functions[i].padding_size < functions[i].frame_size);
        
        // Check total size calculation
        uint64_t calculated_size = functions[i].local_var_size + 
                                   functions[i].saved_regs_size + 
                                   functions[i].padding_size;
        assert(calculated_size <= functions[i].frame_size);
        
        // Frame should be 16-byte aligned
        assert((functions[i].frame_size & 0xF) == 0);
    }
}

// Test 4: Stack Corruption Detection
void test_stack_corruption_detection_aarch64() {
    struct StackCanary {
        uint64_t canary_value;
        uint64_t expected_value;
        bool corrupted;
    };
    
    const int num_canaries = 8;
    StackCanary canaries[num_canaries] = {};
    
    // Setup canaries
    uint64_t good_canary = 0xDEADBEEFCAFEBABE;
    uint64_t bad_canary = 0xBAD0C0FFEEC0FFEE;
    
    for (int i = 0; i < num_canaries; i++) {
        canaries[i].expected_value = good_canary;
        
        if (i == 3 || i == 5) {  // Corrupt these canaries
            canaries[i].canary_value = bad_canary;
            canaries[i].corrupted = true;
        } else {
            canaries[i].canary_value = good_canary;
            canaries[i].corrupted = false;
        }
    }
    
    // Detect corruption
    int corrupted_count = 0;
    for (int i = 0; i < num_canaries; i++) {
        if (canaries[i].canary_value != canaries[i].expected_value) {
            corrupted_count++;
            assert(canaries[i].corrupted);
        } else {
            assert(!canaries[i].corrupted);
        }
    }
    
    assert(corrupted_count == 2);  // Should detect 2 corrupted canaries
}

// Test 5: Stack Pointer Verification
void test_stack_pointer_verification_aarch64() {
    struct StackPointerTest {
        uint64_t stack_pointer;
        uint64_t stack_base;
        uint64_t stack_limit;
        bool should_be_valid;
    };
    
    const int num_tests = 6;
    StackPointerTest tests[num_tests] = {
        // Valid pointers
        {0x70008000, 0x70010000, 0x70000000, true},
        {0x70004000, 0x70010000, 0x70000000, true},
        {0x7000C000, 0x70010000, 0x70000000, true},
        
        // Invalid pointers
        {0x70011000, 0x70010000, 0x70000000, false},  // Above stack
        {0x6FFFF000, 0x70010000, 0x70000000, false},  // Below stack
        {0x00000000, 0x70010000, 0x70000000, false}   // Null pointer
    };
    
    // Verify stack pointers
    for (int i = 0; i < num_tests; i++) {
        bool is_valid = (tests[i].stack_pointer >= tests[i].stack_limit &&
                        tests[i].stack_pointer <= tests[i].stack_base);
        
        assert(is_valid == tests[i].should_be_valid);
        
        if (is_valid) {
            // Valid pointers should be properly aligned
            assert((tests[i].stack_pointer & 0xF) == 0);
        }
    }
}

// Test 6: Return Address Analysis
void test_return_address_analysis_aarch64() {
    struct ReturnAddress {
        uint64_t address;
        bool valid;
        const char* expected_function;
    };
    
    const int num_addresses = 8;
    ReturnAddress addresses[num_addresses] = {
        {0x40001000, true, "main"},
        {0x40001100, true, "process_data"},
        {0x40001200, true, "helper_function"},
        {0x40001300, true, "cleanup"},
        {0x00000000, false, "NULL"},          // Null pointer
        {0xDEADBEEF, false, "INVALID"},       // Invalid address
        {0x40000000, true, "_start"},         // Entry point
        {0x7F00001000, true, "libc_function"} // Library function
    };
    
    // Analyze return addresses
    for (int i = 0; i < num_addresses; i++) {
        uint64_t addr = addresses[i].address;
        
        bool is_valid = true;
        
        // Check for obvious invalid addresses
        if (addr == 0) {
            is_valid = false;
        }
        // Check if in valid code ranges
        else if (addr >= 0x40000000 && addr < 0x50000000) {
            is_valid = true;  // Main executable
        }
        else if (addr >= 0x7F0000000000 && addr < 0x7F1000000000) {
            is_valid = true;  // Shared libraries
        }
        else if (addr >= 0xFFFF000000000000) {
            is_valid = true;  // Kernel
        }
        else {
            is_valid = false;  // Invalid range
        }
        
        assert(is_valid == addresses[i].valid);
        
        if (is_valid) {
            // Valid addresses should be 4-byte aligned
            assert((addr & 0x3) == 0);
        }
    }
}

// Test 7: Stack Trace Generation
void test_stack_trace_generation_aarch64() {
    struct StackTrace {
        int depth;
        uint64_t frames[16];
        uint64_t frame_pointers[16];
    };
    
    const int num_traces = 3;
    StackTrace traces[num_traces] = {
        // Deep trace
        {
            10,
            {0x40001000, 0x40001100, 0x40001200, 0x40001300, 0x40001400,
             0x40001500, 0x40001600, 0x40001700, 0x40001800, 0x40001900},
            {0x7000FFF0, 0x7000FFC0, 0x7000FF90, 0x7000FF60, 0x7000FF30,
             0x7000FF00, 0x7000FED0, 0x7000FEA0, 0x7000FE70, 0x7000FE40}
        },
        // Medium trace
        {
            5,
            {0x40002000, 0x40002100, 0x40002200, 0x40002300, 0x40002400},
            {0x7002FFF0, 0x7002FFC0, 0x7002FF90, 0x7002FF60, 0x7002FF30}
        },
        // Shallow trace
        {
            3,
            {0x40003000, 0x40003100, 0x40003200},
            {0x7004FFF0, 0x7004FFC0, 0x7004FF90}
        }
    };
    
    // Verify stack traces
    for (int t = 0; t < num_traces; t++) {
        assert(traces[t].depth > 0);
        assert(traces[t].depth <= 16);
        
        for (int f = 0; f < traces[t].depth; f++) {
            // Check return addresses
            assert(traces[t].frames[f] >= 0x40000000);
            assert(traces[t].frames[f] < 0x50000000);
            
            // Check frame pointers
            assert(traces[t].frame_pointers[f] >= 0x70000000);
            assert(traces[t].frame_pointers[f] < 0x80000000);
            
            // Frame pointers should be in descending order
            if (f > 0) {
                assert(traces[t].frame_pointers[f] < traces[t].frame_pointers[f-1]);
            }
        }
    }
}

// Test 8: Stack Variable Analysis
void test_stack_variable_analysis_aarch64() {
    struct StackVariable {
        const char* name;
        uint64_t offset;  // Offset from frame pointer
        uint64_t size;
        const char* type;
        bool is_pointer;
    };
    
    const int num_variables = 8;
    StackVariable variables[num_variables] = {
        {"local_int", 0x10, 8, "int64_t", false},
        {"local_ptr", 0x18, 8, "void*", true},
        {"array", 0x20, 32, "char[32]", false},
        {"struct_var", 0x40, 24, "MyStruct", false},
        {"float_val", 0x58, 4, "float", false},
        {"double_val", 0x60, 8, "double", false},
        {"bool_flag", 0x68, 1, "bool", false},
        {"ret_addr", 0x0, 8, "uint64_t", true}  // Return address at FP+0
    };
    
    // Analyze stack variables
    uint64_t total_size = 0;
    for (int i = 0; i < num_variables; i++) {
        assert(variables[i].size > 0);
        assert(variables[i].size <= 256);  // Reasonable max
        assert(strlen(variables[i].name) > 0);
        assert(strlen(variables[i].type) > 0);
        
        // Check alignment
        if (variables[i].size >= 8) {
            assert((variables[i].offset & 0x7) == 0);
        } else if (variables[i].size >= 4) {
            assert((variables[i].offset & 0x3) == 0);
        }
        
        total_size += variables[i].size;
    }
    
    // Total variables should fit in reasonable frame
    assert(total_size <= 0x200);
}

// Test 9: Interrupt Stack Frames
void test_interrupt_stack_frames_aarch64() {
    struct InterruptFrame {
        uint64_t elr_el1;      // Exception Link Register
        uint64_t spsr_el1;     // Saved Processor State
        uint64_t sp_el0;       // User Stack Pointer
        uint64_t x[31];        // General registers
        uint64_t q[32][2];     // Vector registers (128-bit)
        uint64_t fpcr;         // Floating-point Control Register
        uint64_t fpsr;         // Floating-point Status Register
        uint64_t tpidr_el0;    // Thread ID Register
        uint64_t far_el1;      // Fault Address Register
        uint64_t esr_el1;      // Exception Syndrome Register
    };
    
    InterruptFrame int_frame = {};
    
    // Setup interrupt frame
    int_frame.elr_el1 = 0x40001000;
    int_frame.spsr_el1 = 0x60000000;  // EL1, DAIF masked
    int_frame.sp_el0 = 0x70010000;
    int_frame.far_el1 = 0xDEADBEEF;
    int_frame.esr_el1 = 0x96000047;  // Data abort, write, EL1
    
    // Set some registers
    for (int i = 0; i < 31; i++) {
        int_frame.x[i] = 0x1000 + i * 8;
    }
    
    // Set vector registers
    for (int i = 0; i < 32; i++) {
        int_frame.q[i][0] = 0xAAAAAAAAAAAAAAAA;
        int_frame.q[i][1] = 0xBBBBBBBBBBBBBBBB;
    }
    
    // Verify interrupt frame
    assert(int_frame.elr_el1 >= 0x40000000);
    assert(int_frame.sp_el0 < 0x8000000000);
    assert((int_frame.spsr_el1 & 0xF) == 0);  // EL1
    assert(int_frame.far_el1 == 0xDEADBEEF);
    assert((int_frame.esr_el1 & 0xFC000000) == 0x96000000);
    
    // Check register values
    assert(int_frame.x[0] == 0x1000);
    assert(int_frame.x[30] == 0x10F0);
}

// Test 10: Stack Overflow Detection
void test_stack_overflow_detection_aarch64() {
    struct StackUsage {
        uint64_t stack_pointer;
        uint64_t stack_base;
        uint64_t stack_limit;
        uint64_t stack_usage;
        bool overflow;
    };
    
    const int num_cases = 6;
    StackUsage cases[num_cases] = {
        // Normal usage
        {0x70008000, 0x70010000, 0x70000000, 0x8000, false},
        {0x70004000, 0x70010000, 0x70000000, 0xC000, false},
        
        // High usage
        {0x70001000, 0x70010000, 0x70000000, 0xF000, false},
        
        // At limit
        {0x70000100, 0x70010000, 0x70000000, 0xFF00, false},
        
        // Overflow cases
        {0x70000000, 0x70010000, 0x70000000, 0x10000, true},
        {0x6FFFFFF0, 0x70010000, 0x70000000, 0x10010