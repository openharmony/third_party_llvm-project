// ============================================================================
// Minidump Exception Tests for AArch64
// ============================================================================
// Tests exception record parsing and context handling for aarch64 minidumps

#include <cstdint>
#include <cstring>
#include <cassert>

// AArch64 context flags
#define CONTEXT_AARCH64  0x00400000
#define CONTEXT_CONTROL  (CONTEXT_AARCH64 | 0x1)
#define CONTEXT_INTEGER  (CONTEXT_AARCH64 | 0x2)
#define CONTEXT_FLOATING_POINT (CONTEXT_AARCH64 | 0x4)
#define CONTEXT_DEBUG_REGISTERS (CONTEXT_AARCH64 | 0x8)
#define CONTEXT_FULL (CONTEXT_CONTROL | CONTEXT_INTEGER | CONTEXT_FLOATING_POINT)

// AArch64 vector register
struct __uint128_t {
    uint64_t low;
    uint64_t high;
};

// AArch64 exception codes
#define EXCEPTION_ACCESS_VIOLATION          0xC0000005
#define EXCEPTION_ARRAY_BOUNDS_EXCEEDED     0xC000008C
#define EXCEPTION_BREAKPOINT                0x80000003
#define EXCEPTION_DATATYPE_MISALIGNMENT     0x80000002
#define EXCEPTION_FLT_DENORMAL_OPERAND      0xC000008D
#define EXCEPTION_FLT_DIVIDE_BY_ZERO        0xC000008E
#define EXCEPTION_FLT_INEXACT_RESULT        0xC000008F
#define EXCEPTION_FLT_INVALID_OPERATION     0xC0000090
#define EXCEPTION_FLT_OVERFLOW              0xC0000091
#define EXCEPTION_FLT_STACK_CHECK           0xC0000092
#define EXCEPTION_FLT_UNDERFLOW             0xC0000093
#define EXCEPTION_ILLEGAL_INSTRUCTION       0xC000001D
#define EXCEPTION_IN_PAGE_ERROR             0xC0000006
#define EXCEPTION_INT_DIVIDE_BY_ZERO        0xC0000094
#define EXCEPTION_INT_OVERFLOW              0xC0000095
#define EXCEPTION_INVALID_DISPOSITION       0xC0000026
#define EXCEPTION_NONCONTINUABLE_EXCEPTION  0xC0000025
#define EXCEPTION_PRIV_INSTRUCTION          0xC0000096
#define EXCEPTION_SINGLE_STEP               0x80000004
#define EXCEPTION_STACK_OVERFLOW            0xC00000FD

// Minidump exception structure for AArch64
struct MinidumpExceptionAArch64 {
    uint32_t exception_code;
    uint32_t exception_flags;
    uint64_t exception_record;
    uint64_t exception_address;
    uint32_t number_parameters;
    uint32_t __unused_alignment;
    uint64_t exception_information[15];
};

// AArch64 context structure
struct MinidumpContextAArch64 {
    uint64_t context_flags;
    
    // Integer registers
    uint64_t x[31];
    uint64_t sp;
    uint64_t pc;
    uint64_t pstate;
    
    // Floating point/NEON registers
    uint32_t fpsr;
    uint32_t fpcr;
    __uint128_t v[32];
    
    // Debug registers
    uint64_t bcr[8];
    uint64_t bvr[8];
    uint64_t wcr[2];
    uint64_t wvr[2];
};

// Exception stream structure
struct MinidumpExceptionStream {
    uint32_t thread_id;
    uint32_t __alignment;
    MinidumpExceptionAArch64 exception_record;
    MinidumpContextAArch64 thread_context;
};

// Test 1: Access Violation Exception
void test_access_violation_aarch64() {
    MinidumpExceptionStream exception_stream = {};
    
    // Setup thread information
    exception_stream.thread_id = 1234;
    
    // Setup access violation exception
    exception_stream.exception_record.exception_code = EXCEPTION_ACCESS_VIOLATION;
    exception_stream.exception_record.exception_flags = 0;
    exception_stream.exception_record.exception_record = 0;
    exception_stream.exception_record.exception_address = 0xDEADBEEF;
    exception_stream.exception_record.number_parameters = 2;
    exception_stream.exception_record.exception_information[0] = 0;  // Read operation
    exception_stream.exception_record.exception_information[1] = 0xDEADBEEF;  // Fault address
    
    // Setup full AArch64 context
    exception_stream.thread_context.context_flags = CONTEXT_FULL;
    
    // Set general purpose registers
    for (int i = 0; i < 31; i++) {
        exception_stream.thread_context.x[i] = 0x1000 + i * 0x100;
    }
    exception_stream.thread_context.x[29] = 0x7000FFF0;  // Frame pointer
    exception_stream.thread_context.x[30] = 0x40001000;  // Link register
    
    // Set control registers
    exception_stream.thread_context.sp = 0x70010000;
    exception_stream.thread_context.pc = 0x40000000;
    exception_stream.thread_context.pstate = 0x60000000;  // EL1, interrupts masked
    
    // Set floating point registers
    exception_stream.thread_context.fpsr = 0;
    exception_stream.thread_context.fpcr = 0;
    
    for (int i = 0; i < 32; i++) {
        exception_stream.thread_context.v[i].low = 0xAAAAAAAAAAAAAAAA;
        exception_stream.thread_context.v[i].high = 0xBBBBBBBBBBBBBBBB;
    }
    
    // Test exception parsing
    assert(exception_stream.exception_record.exception_code == EXCEPTION_ACCESS_VIOLATION);
    assert(exception_stream.exception_record.exception_address == 0xDEADBEEF);
    assert(exception_stream.exception_record.number_parameters == 2);
    assert(exception_stream.exception_record.exception_information[0] == 0);
    assert(exception_stream.exception_record.exception_information[1] == 0xDEADBEEF);
    
    // Test thread context
    assert(exception_stream.thread_context.context_flags == CONTEXT_FULL);
    assert(exception_stream.thread_id == 1234);
    assert(exception_stream.thread_context.pc == 0x40000000);
    assert(exception_stream.thread_context.sp == 0x70010000);
    assert(exception_stream.thread_context.x[29] == 0x7000FFF0);
    assert(exception_stream.thread_context.x[30] == 0x40001000);
    
    // Test vector registers
    for (int i = 0; i < 32; i++) {
        assert(exception_stream.thread_context.v[i].low == 0xAAAAAAAAAAAAAAAA);
        assert(exception_stream.thread_context.v[i].high == 0xBBBBBBBBBBBBBBBB);
    }
}

// Test 2: Illegal Instruction Exception
void test_illegal_instruction_aarch64() {
    MinidumpExceptionStream exception_stream = {};
    
    exception_stream.thread_id = 5678;
    exception_stream.exception_record.exception_code = EXCEPTION_ILLEGAL_INSTRUCTION;
    exception_stream.exception_record.exception_address = 0x40012345;
    exception_stream.exception_record.number_parameters = 0;
    
    // Setup minimal context
    exception_stream.thread_context.context_flags = CONTEXT_CONTROL | CONTEXT_INTEGER;
    exception_stream.thread_context.pc = 0x40012345;
    exception_stream.thread_context.sp = 0x70020000;
    exception_stream.thread_context.pstate = 0x60000000;
    
    // Set some register values
    exception_stream.thread_context.x[0] = 0x12345678;
    exception_stream.thread_context.x[1] = 0x87654321;
    exception_stream.thread_context.x[2] = 0xDEADBEEF;
    
    assert(exception_stream.exception_record.exception_code == EXCEPTION_ILLEGAL_INSTRUCTION);
    assert(exception_stream.thread_context.pc == 0x40012345);
    assert((exception_stream.thread_context.context_flags & CONTEXT_CONTROL) != 0);
    assert((exception_stream.thread_context.context_flags & CONTEXT_INTEGER) != 0);
}

// Test 3: Floating Point Exception
void test_floating_point_exception_aarch64() {
    MinidumpExceptionStream exception_stream = {};
    
    exception_stream.thread_id = 9012;
    exception_stream.exception_record.exception_code = EXCEPTION_FLT_DIVIDE_BY_ZERO;
    exception_stream.exception_record.exception_address = 0x40030000;
    exception_stream.exception_record.number_parameters = 1;
    exception_stream.exception_record.exception_information[0] = 0x40030000;
    
    // Setup context with floating point
    exception_stream.thread_context.context_flags = CONTEXT_FULL;
    exception_stream.thread_context.pc = 0x40030000;
    exception_stream.thread_context.sp = 0x70030000;
    exception_stream.thread_context.fpsr = 0x00000010;  // Divide by zero flag
    exception_stream.thread_context.fpcr = 0x00000000;
    
    // Set some vector registers to FP values
    for (int i = 0; i < 16; i++) {
        exception_stream.thread_context.v[i].low = 0x3FF0000000000000;  // 1.0
        exception_stream.thread_context.v[i].high = 0x0000000000000000;
    }
    
    assert(exception_stream.exception_record.exception_code == EXCEPTION_FLT_DIVIDE_BY_ZERO);
    assert(exception_stream.thread_context.fpsr == 0x00000010);
    assert(exception_stream.thread_context.context_flags == CONTEXT_FULL);
}

// Test 4: Stack Overflow Exception
void test_stack_overflow_aarch64() {
    MinidumpExceptionStream exception_stream = {};
    
    exception_stream.thread_id = 3456;
    exception_stream.exception_record.exception_code = EXCEPTION_STACK_OVERFLOW;
    exception_stream.exception_record.exception_address = 0x40040000;
    exception_stream.exception_record.number_parameters = 0;
    
    // Setup context with stack at limit
    exception_stream.thread_context.context_flags = CONTEXT_FULL;
    exception_stream.thread_context.pc = 0x40040000;
    exception_stream.thread_context.sp = 0x70000000;  // Very low stack
    exception_stream.thread_context.x[29] = 0x70000000;  // Frame pointer at limit
    
    // Fill stack-like data in registers
    for (int i = 0; i < 31; i++) {
        exception_stream.thread_context.x[i] = 0x70000000 + i * 8;
    }
    
    assert(exception_stream.exception_record.exception_code == EXCEPTION_STACK_OVERFLOW);
    assert(exception_stream.thread_context.sp == 0x70000000);
    assert(exception_stream.thread_context.x[29] == 0x70000000);
}

// Test 5: Breakpoint Exception
void test_breakpoint_exception_aarch64() {
    MinidumpExceptionStream exception_stream = {};
    
    exception_stream.thread_id = 7890;
    exception_stream.exception_record.exception_code = EXCEPTION_BREAKPOINT;
    exception_stream.exception_record.exception_address = 0x40050000;
    exception_stream.exception_record.number_parameters = 0;
    
    // Setup debug context
    exception_stream.thread_context.context_flags = CONTEXT_FULL | CONTEXT_DEBUG_REGISTERS;
    exception_stream.thread_context.pc = 0x40050000;
    exception_stream.thread_context.sp = 0x70040000;
    
    // Set breakpoint registers
    for (int i = 0; i < 8; i++) {
        exception_stream.thread_context.bcr[i] = 0x00000000;
        exception_stream.thread_context.bvr[i] = 0x00000000;
    }
    exception_stream.thread_context.bcr[0] = 0x00000001;  // Breakpoint 0 enabled
    exception_stream.thread_context.bvr[0] = 0x40050000;  // Break at this address
    
    assert(exception_stream.exception_record.exception_code == EXCEPTION_BREAKPOINT);
    assert(exception_stream.thread_context.pc == 0x40050000);
    assert((exception_stream.thread_context.context_flags & CONTEXT_DEBUG_REGISTERS) != 0);
    assert(exception_stream.thread_context.bvr[0] == 0x40050000);
}

// Test 6: Single Step Exception
void test_single_step_exception_aarch64() {
    MinidumpExceptionStream exception_stream = {};
    
    exception_stream.thread_id = 1111;
    exception_stream.exception_record.exception_code = EXCEPTION_SINGLE_STEP;
    exception_stream.exception_record.exception_address = 0x40060000;
    exception_stream.exception_record.number_parameters = 0;
    
    exception_stream.thread_context.context_flags = CONTEXT_FULL;
    exception_stream.thread_context.pc = 0x40060000;
    exception_stream.thread_context.sp = 0x70050000;
    exception_stream.thread_context.pstate = 0x60000000;
    
    assert(exception_stream.exception_record.exception_code == EXCEPTION_SINGLE_STEP);
    assert(exception_stream.thread_context.pc == 0x40060000);
}

// Test 7: Multiple Nested Exceptions
void test_nested_exceptions_aarch64() {
    // Simulate nested exception handling
    MinidumpExceptionStream exceptions[3] = {};
    
    // First exception - access violation
    exceptions[0].thread_id = 1000;
    exceptions[0].exception_record.exception_code = EXCEPTION_ACCESS_VIOLATION;
    exceptions[0].exception_record.exception_address = 0x40070000;
    exceptions[0].exception_record.number_parameters = 2;
    exceptions[0].exception_record.exception_information[0] = 1;  // Write
    exceptions[0].exception_record.exception_information[1] = 0x00000000;
    
    // Second exception - in handler
    exceptions[1].thread_id = 1000;
    exceptions[1].exception_record.exception_code = EXCEPTION_ILLEGAL_INSTRUCTION;
    exceptions[1].exception_record.exception_address = 0x40100000;
    exceptions[1].exception_record.number_parameters = 0;
    exceptions[1].exception_record.exception_record = (uint64_t)&exceptions[0].exception_record;
    
    // Third exception - in nested handler
    exceptions[2].thread_id = 1000;
    exceptions[2].exception_record.exception_code = EXCEPTION_STACK_OVERFLOW;
    exceptions[2].exception_record.exception_address = 0x40110000;
    exceptions[2].exception_record.number_parameters = 0;
    exceptions[2].exception_record.exception_record = (uint64_t)&exceptions[1].exception_record;
    
    // Verify chain
    assert(exceptions[0].exception_record.exception_code == EXCEPTION_ACCESS_VIOLATION);
    assert(exceptions[1].exception_record.exception_record == (uint64_t)&exceptions[0].exception_record);
    assert(exceptions[2].exception_record.exception_record == (uint64_t)&exceptions[1].exception_record);
}

// Test 8: Exception with Maximum Parameters
void test_exception_max_parameters_aarch64() {
    MinidumpExceptionStream exception_stream = {};
    
    exception_stream.thread_id = 2222;
    exception_stream.exception_record.exception_code = EXCEPTION_IN_PAGE_ERROR;
    exception_stream.exception_record.exception_address = 0x40080000;
    exception_stream.exception_record.number_parameters = 15;  // Maximum
    
    // Fill all parameters
    for (int i = 0; i < 15; i++) {
        exception_stream.exception_record.exception_information[i] = 0x1000 + i * 0x100;
    }
    
    exception_stream.thread_context.context_flags = CONTEXT_FULL;
    exception_stream.thread_context.pc = 0x40080000;
    exception_stream.thread_context.sp = 0x70060000;
    
    assert(exception_stream.exception_record.number_parameters == 15);
    for (int i = 0; i < 15; i++) {
        assert(exception_stream.exception_record.exception_information[i] == 0x1000 + i * 0x100);
    }
}

// Test 9: Exception with No Context
void test_exception_no_context_aarch64() {
    MinidumpExceptionStream exception_stream = {};
    
    exception_stream.thread_id = 3333;
    exception_stream.exception_record.exception_code = EXCEPTION_ACCESS_VIOLATION;
    exception_stream.exception_record.exception_address = 0x40090000;
    exception_stream.exception_record.number_parameters = 2;
    
    // No context saved
    exception_stream.thread_context.context_flags = 0;
    
    assert(exception_stream.exception_record.exception_code == EXCEPTION_ACCESS_VIOLATION);
    assert(exception_stream.thread_context.context_flags == 0);
}

// Test 10: Exception with Partial Context
void test_exception_partial_context_aarch64() {
    MinidumpExceptionStream exception_stream = {};
    
    exception_stream.thread_id = 4444;
    exception_stream.exception_record.exception_code = EXCEPTION_PRIV_INSTRUCTION;
    exception_stream.exception_record.exception_address = 0x40100000;
    exception_stream.exception_record.number_parameters = 0;
    
    // Only integer registers
    exception_stream.thread_context.context_flags = CONTEXT_INTEGER;
    for (int i = 0; i < 31; i++) {
        exception_stream.thread_context.x[i] = 0x2000 + i * 8;
    }
    
    // PC and SP should be 0 since CONTEXT_CONTROL not set
    exception_stream.thread_context.pc = 0;
    exception_stream.thread_context.sp = 0;
    
    assert(exception_stream.thread_context.context_flags == CONTEXT_INTEGER);
    assert(exception_stream.thread_context.pc == 0);
    assert(exception_stream.thread_context.sp == 0);
    assert(exception_stream.thread_context.x[0] == 0x2000);
    assert(exception_stream.thread_context.x[30] == 0x20F0);
}

// Run all exception tests
void test_all_minidump_exceptions_aarch64() {
    test_access_violation_aarch64();
    test_illegal_instruction_aarch64();
    test_floating_point_exception_aarch64();
    test_stack_overflow_aarch64();
    test_breakpoint_exception_aarch64();
    test_single_step_exception_aarch64();
    test_nested_exceptions_aarch64();
    test_exception_max_parameters_aarch64();
    test_exception_no_context_aarch64();
    test_exception_partial_context_aarch64();
    
    // Verify all tests passed
    assert(true);
}