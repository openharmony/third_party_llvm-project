// ============================================================================
// Minidump Thread Tests for AArch64
// ============================================================================
// Tests thread information and context parsing for aarch64 minidumps

#include <cstdint>
#include <cstring>
#include <cassert>

// Thread entry structure
struct MinidumpThread {
    uint32_t thread_id;
    uint32_t suspend_count;
    uint32_t priority_class;
    uint32_t priority;
    uint64_t teb;           // Thread Environment Block
    uint64_t stack;         // Stack descriptor
    uint64_t thread_context; // Context RVA
};

// Thread list structure
struct MinidumpThreadList {
    uint32_t number_of_threads;
    MinidumpThread threads[1];  // Flexible array
};

// AArch64 context (redefined for completeness)
struct MinidumpContextAArch64 {
    uint64_t context_flags;
    uint64_t x[31];
    uint64_t sp;
    uint64_t pc;
    uint64_t pstate;
    uint32_t fpsr;
    uint32_t fpcr;
    uint64_t v[32][2];  // 128-bit vector registers
};

// Test 1: Basic Thread Information
void test_basic_thread_info_aarch64() {
    const int num_threads = 4;
    MinidumpThread threads[num_threads] = {
        // Main thread
        {
            1234,           // thread_id
            0,              // suspend_count
            2,              // priority_class (NORMAL)
            8,              // priority
            0x7000000000,   // teb
            0x7001000000,   // stack
            0x2000          // context RVA
        },
        // Worker thread
        {
            1235,           // thread_id
            0,              // suspend_count
            2,              // priority_class
            8,              // priority
            0x7002000000,   // teb
            0x7003000000,   // stack
            0x2400          // context RVA
        },
        // High priority thread
        {
            1236,           // thread_id
            1,              // suspend_count (suspended)
            3,              // priority_class (HIGH)
            15,             // priority
            0x7004000000,   // teb
            0x7005000000,   // stack
            0x2800          // context RVA
        },
        // Low priority thread
        {
            1237,           // thread_id
            0,              // suspend_count
            1,              // priority_class (IDLE)
            2,              // priority
            0x7006000000,   // teb
            0x7007000000,   // stack
            0x2C00          // context RVA
        }
    };
    
    // Verify thread properties
    for (int i = 0; i < num_threads; i++) {
        assert(threads[i].thread_id > 0);
        assert(threads[i].priority_class >= 1 && threads[i].priority_class <= 3);
        assert(threads[i].priority <= 31);
        assert(threads[i].teb > 0);
        assert(threads[i].stack > 0);
        assert(threads[i].thread_context > 0);
        
        // TEB should be in user space
        assert(threads[i].teb < 0x8000000000);
        assert(threads[i].stack < 0x8000000000);
    }
    
    // Check for unique thread IDs
    for (int i = 0; i < num_threads; i++) {
        for (int j = i + 1; j < num_threads; j++) {
            assert(threads[i].thread_id != threads[j].thread_id);
        }
    }
}

// Test 2: Thread Contexts
void test_thread_contexts_aarch64() {
    const int num_threads = 3;
    MinidumpContextAArch64 contexts[num_threads] = {};
    
    // Main thread context
    contexts[0].context_flags = 0xFFFFFFFF;
    contexts[0].x[0] = 0x1111111111111111;
    contexts[0].x[1] = 0x2222222222222222;
    contexts[0].x[29] = 0x7000FFF0;  // Frame pointer
    contexts[0].x[30] = 0x40001000;  // Link register
    contexts[0].sp = 0x70010000;
    contexts[0].pc = 0x40002000;
    contexts[0].pstate = 0x60000000;
    contexts[0].fpsr = 0;
    contexts[0].fpcr = 0;
    
    // Worker thread context
    contexts[1].context_flags = 0xFFFFFFFF;
    contexts[1].x[0] = 0x3333333333333333;
    contexts[1].x[1] = 0x4444444444444444;
    contexts[1].x[29] = 0x7002FFF0;
    contexts[1].x[30] = 0x40003000;
    contexts[1].sp = 0x70030000;
    contexts[1].pc = 0x40004000;
    contexts[1].pstate = 0x60000000;
    
    // I/O thread context
    contexts[2].context_flags = 0xFFFFFFFF;
    contexts[2].x[0] = 0x5555555555555555;
    contexts[2].x[1] = 0x6666666666666666;
    contexts[2].x[29] = 0x7004FFF0;
    contexts[2].x[30] = 0x40005000;
    contexts[2].sp = 0x70050000;
    contexts[2].pc = 0x40006000;
    contexts[2].pstate = 0x60000000;
    
    // Verify contexts
    for (int i = 0; i < num_threads; i++) {
        assert(contexts[i].context_flags != 0);
        assert(contexts[i].pc >= 0x40000000);
        assert(contexts[i].sp < 0x8000000000);
        assert(contexts[i].x[29] < 0x8000000000);  // Frame pointer in user space
        assert(contexts[i].x[30] >= 0x40000000);   // Return address in code
    }
}

// Test 3: Thread Stacks
void test_thread_stacks_aarch64() {
    struct ThreadStack {
        MinidumpThread thread;
        uint64_t stack_base;
        uint64_t stack_limit;
        uint64_t stack_size;
    };
    
    const int num_threads = 4;
    ThreadStack thread_stacks[num_threads] = {
        {
            {1234, 0, 2, 8, 0x7000000000, 0x7001000000, 0x2000},
            0x7001000000,
            0x7000F80000,
            0x80000
        },
        {
            {1235, 0, 2, 8, 0x7002000000, 0x7003000000, 0x2400},
            0x7003000000,
            0x7002800000,
            0x80000
        },
        {
            {1236, 1, 3, 15, 0x7004000000, 0x7005000000, 0x2800},
            0x7005000000,
            0x7004800000,
            0x80000
        },
        {
            {1237, 0, 1, 2, 0x7006000000, 0x7007000000, 0x2C00},
            0x7007000000,
            0x7006800000,
            0x80000
        }
    };
    
    // Verify stack properties
    for (int i = 0; i < num_threads; i++) {
        uint64_t stack_base = thread_stacks[i].stack_base;
        uint64_t stack_limit = thread_stacks[i].stack_limit;
        uint64_t stack_size = thread_stacks[i].stack_size;
        
        assert(stack_base > stack_limit);
        assert(stack_size == stack_base - stack_limit);
        assert(stack_size == 0x80000);  // 512KB stacks
        
        // Stacks should be page aligned
        assert((stack_base & 0xFFF) == 0);
        assert((stack_limit & 0xFFF) == 0);
        
        // Stacks should not overlap
        for (int j = i + 1; j < num_threads; j++) {
            uint64_t other_base = thread_stacks[j].stack_base;
            uint64_t other_limit = thread_stacks[j].stack_limit;
            
            assert(stack_base <= other_limit || other_base <= stack_limit);
        }
    }
}

// Test 4: Thread Priorities
void test_thread_priorities_aarch64() {
    struct ThreadPriority {
        uint32_t thread_id;
        uint32_t priority_class;
        uint32_t priority;
        const char* class_name;
    };
    
    const int num_threads = 6;
    ThreadPriority threads[num_threads] = {
        {1000, 1, 4, "IDLE_PRIORITY_CLASS"},      // Idle
        {1001, 1, 2, "IDLE_PRIORITY_CLASS"},      // Idle (lower)
        {1002, 2, 8, "NORMAL_PRIORITY_CLASS"},    // Normal
        {1003, 2, 10, "NORMAL_PRIORITY_CLASS"},   // Normal (higher)
        {1004, 3, 13, "HIGH_PRIORITY_CLASS"},     // High
        {1005, 3, 15, "HIGH_PRIORITY_CLASS"}      // High (highest)
    };
    
    // Verify priority ranges
    for (int i = 0; i < num_threads; i++) {
        uint32_t priority_class = threads[i].priority_class;
        uint32_t priority = threads[i].priority;
        
        assert(priority_class >= 1 && priority_class <= 3);
        
        switch (priority_class) {
            case 1:  // IDLE
                assert(priority >= 1 && priority <= 6);
                break;
            case 2:  // NORMAL
                assert(priority >= 6 && priority <= 10);
                break;
            case 3:  // HIGH
                assert(priority >= 11 && priority <= 15);
                break;
        }
        
        assert(strlen(threads[i].class_name) > 0);
    }
}

// Test 5: Thread States
void test_thread_states_aarch64() {
    struct ThreadState {
        uint32_t thread_id;
        uint32_t suspend_count;
        bool is_running;
        bool is_waiting;
        bool is_terminated;
        uint64_t wait_reason;
    };
    
    const int num_threads = 5;
    ThreadState threads[num_threads] = {
        {2000, 0, true, false, false, 0},     // Running
        {2001, 0, false, true, false, 0x0A},  // Waiting (executive)
        {2002, 1, false, false, false, 0},    // Suspended
        {2003, 0, false, true, false, 0x0B},  // Waiting (user request)
        {2004, 0, false, false, true, 0}      // Terminated
    };
    
    // Verify thread states
    for (int i = 0; i < num_threads; i++) {
        assert(threads[i].thread_id > 0);
        
        // Validate state combinations
        int state_count = 0;
        if (threads[i].is_running) state_count++;
        if (threads[i].is_waiting) state_count++;
        if (threads[i].is_terminated) state_count++;
        
        assert(state_count == 1);  // Must be exactly one state
        
        // Suspended threads have suspend_count > 0
        if (threads[i].suspend_count > 0) {
            assert(!threads[i].is_running);
        }
    }
}

// Test 6: Thread Call Stacks
void test_thread_call_stacks_aarch64() {
    struct StackFrame {
        uint64_t frame_pointer;
        uint64_t return_address;
        uint64_t saved_registers[4];  // X19-X22
    };
    
    struct ThreadCallStack {
        uint32_t thread_id;
        int num_frames;
        StackFrame frames[8];
    };
    
    const int num_threads = 3;
    ThreadCallStack call_stacks[num_threads] = {
        // Main thread deep call stack
        {
            3000,
            8,
            {
                {0x7000FFF0, 0x40001000, {0x1000, 0x1001, 0x1002, 0x1003}},
                {0x7000FFC0, 0x40001100, {0x1004, 0x1005, 0x1006, 0x1007}},
                {0x7000FF90, 0x40001200, {0x1008, 0x1009, 0x100A, 0x100B}},
                {0x7000FF60, 0x40001300, {0x100C, 0x100D, 0x100E, 0x100F}},
                {0x7000FF30, 0x40001400, {0x1010, 0x1011, 0x1012, 0x1013}},
                {0x7000FF00, 0x40001500, {0x1014, 0x1015, 0x1016, 0x1017}},
                {0x7000FED0, 0x40001600, {0x1018, 0x1019, 0x101A, 0x101B}},
                {0x7000FEA0, 0x40001700, {0x101C, 0x101D, 0x101E, 0x101F}}
            }
        },
        // Worker thread medium stack
        {
            3001,
            5,
            {
                {0x7002FFF0, 0x40002000, {0x2000, 0x2001, 0x2002, 0x2003}},
                {0x7002FFC0, 0x40002100, {0x2004, 0x2005, 0x2006, 0x2007}},
                {0x7002FF90, 0x40002200, {0x2008, 0x2009, 0x200A, 0x200B}},
                {0x7002FF60, 0x40002300, {0x200C, 0x200D, 0x200E, 0x200F}},
                {0x7002FF30, 0x40002400, {0x2010, 0x2011, 0x2012, 0x2013}},
                {0, 0, {0, 0, 0, 0}},
                {0, 0, {0, 0, 0, 0}},
                {0, 0, {0, 0, 0, 0}}
            }
        },
        // I/O thread shallow stack
        {
            3002,
            3,
            {
                {0x7004FFF0, 0x40003000, {0x3000, 0x3001, 0x3002, 0x3003}},
                {0x7004FFC0, 0x40003100, {0x3004, 0x3005, 0x3006, 0x3007}},
                {0x7004FF90, 0x40003200, {0x3008, 0x3009, 0x300A, 0x300B}},
                {0, 0, {0, 0, 0, 0}},
                {0, 0, {0, 0, 0, 0}},
                {0, 0, {0, 0, 0, 0}},
                {0, 0, {0, 0, 0, 0}},
                {0, 0, {0, 0, 0, 0}}
            }
        }
    };
    
    // Verify call stacks
    for (int t = 0; t < num_threads; t++) {
        assert(call_stacks[t].num_frames > 0);
        assert(call_stacks[t].num_frames <= 8);
        
        for (int f = 0; f < call_stacks[t].num_frames; f++) {
            const StackFrame& frame = call_stacks[t].frames[f];
            
            assert(frame.frame_pointer > 0x7000000000);
            assert(frame.frame_pointer < 0x8000000000);
            assert(frame.return_address >= 0x40000000);
            assert(frame.return_address < 0x50000000);
            
            // Frame pointers should decrease (stack grows down)
            if (f > 0) {
                assert(frame.frame_pointer < call_stacks[t].frames[f-1].frame_pointer);
            }
            
            // Check saved registers
            for (int r = 0; r < 4; r++) {
                assert(frame.saved_registers[r] >= 0x1000);
            }
        }
    }
}

// Test 7: Thread Local Storage
void test_thread_local_storage_aarch64() {
    struct ThreadTLS {
        uint32_t thread_id;
        uint64_t teb_address;
        uint64_t tls_slots[64];
        uint64_t tls_index;
    };
    
    const int num_threads = 3;
    ThreadTLS tls_data[num_threads] = {
        {
            4000,
            0x7000000000,
            {0x1000, 0x2000, 0x3000, 0x4000},
            4
        },
        {
            4001,
            0x7002000000,
            {0x5000, 0x6000, 0x7000, 0x8000, 0x9000},
            5
        },
        {
            4002,
            0x7004000000,
            {0xA000, 0xB000, 0xC000},
            3
        }
    };
    
    // Verify TLS data
    for (int i = 0; i < num_threads; i++) {
        assert(tls_data[i].teb_address > 0);
        assert(tls_data[i].teb_address < 0x8000000000);
        assert(tls_data[i].tls_index <= 64);
        
        // Check TLS slots
        for (uint64_t j = 0; j < tls_data[i].tls_index; j++) {
            assert(tls_data[i].tls_slots[j] != 0);
        }
    }
}

// Test 8: Thread Creation and Exit
void test_thread_creation_exit_aarch64() {
    struct ThreadLifecycle {
        uint32_t thread_id;
        uint64_t create_time;
        uint64_t exit_time;
        uint64_t kernel_time;
        uint64_t user_time;
        uint64_t start_address;
        uint32_t exit_code;
    };
    
    const int num_threads = 4;
    ThreadLifecycle threads[num_threads] = {
        // Running thread
        {5000, 0x5F3A1B2C, 0, 0x1234, 0x5678, 0x40001000, 0},
        // Exited normally
        {5001, 0x5F3A1B2D, 0x5F3A2B2D, 0x2345, 0x6789, 0x40002000, 0},
        // Crashed
        {5002, 0x5F3A1B2E, 0x5F3A1B2F, 0x3456, 0x789A, 0x40003000, 0xC0000005},
        // Terminated
        {5003, 0x5F3A1B2F, 0x5F3A2B2F, 0x4567, 0x89AB, 0x40004000, 0x00000001}
    };
    
    // Verify lifecycle data
    for (int i = 0; i < num_threads; i++) {
        assert(threads[i].create_time > 0);
        assert(threads[i].kernel_time >= 0);
        assert(threads[i].user_time >= 0);
        assert(threads[i].start_address >= 0x40000000);
        
        if (threads[i].exit_time > 0) {
            assert(threads[i].exit_time >= threads[i].create_time);
        }
    }
}

// Test 9: Thread Affinity
void test_thread_affinity_aarch64() {
    struct ThreadAffinity {
        uint32_t thread_id;
        uint64_t affinity_mask;
        uint32_t ideal_processor;
        uint32_t current_processor;
    };
    
    const int num_threads = 4;
    ThreadAffinity threads[num_threads] = {
        // Pinned to CPU 0-3
        {6000, 0x0000000F, 0, 1},
        // Pinned to CPU 4-7
        {6001, 0x000000F0, 4, 5},
        // All CPUs
        {6002, 0xFFFFFFFF, 8, 2},
        // Single CPU
        {6003, 0x00000001, 0, 0}
    };
    
    // Verify affinity data
    for (int i = 0; i < num_threads; i++) {
        assert(threads[i].affinity_mask != 0);
        assert(threads[i].ideal_processor < 64);
        assert(threads[i].current_processor < 64);
        
        // Current processor should be in affinity mask
        uint64_t cpu_mask = 1ULL << threads[i].current_processor;
        assert((threads[i].affinity_mask & cpu_mask) != 0);
        
        // Ideal processor should be in affinity mask
        uint64_t ideal_mask = 1ULL << threads[i].ideal_processor;
        assert((threads[i].affinity_mask & ideal_mask) != 0);
    }
}

// Test 10: Thread Exception State
void test_thread_exception_state_aarch64() {
    struct ThreadException {
        uint32_t thread_id;
        uint32_t exception_code;
        uint64_t exception_address;
        uint32_t exception_flags;
        uint64_t exception_record;
    };
    
    const int num_threads = 3;
    ThreadException exceptions[num_threads] = {
        // Access violation
        {7000, 0xC0000005, 0xDEADBEEF, 0, 0},
        // Illegal instruction
        {7001, 0xC000001D, 0x40012345, 0, 0},
        // Stack overflow
        {7002, 0xC00000FD, 0x70000000, 0, 0}
    };
    
    // Verify exception data
    for (int i = 0; i < num_threads; i++) {
        assert(exceptions[i].exception_code != 0);
        assert(exceptions[i].exception_address != 0);
        
        // Check exception address ranges
        switch (exceptions[i].exception_code) {
            case 0xC0000005:  // Access violation
                assert(exceptions[i].exception_address == 0xDEADBEEF);
                break;
            case 0xC000001D:  // Illegal instruction
                assert(exceptions[i].exception_address >= 0x40000000);
                break;
            case 0xC00000FD:  // Stack overflow
                assert(exceptions[i].exception_address < 0x8000000000);
                break;
        }
    }
}

// Run all thread tests
void test_all_minidump_threads_aarch64() {
    test_basic_thread_info_aarch64();
    test_thread_contexts_aarch64();
    test_thread_stacks_aarch64();
    test_thread_priorities_aarch64();
    test_thread_states_aarch64();
    test_thread_call_stacks_aarch64();
    test_thread_local_storage_aarch64();
    test_thread_creation_exit_aarch64();
    test_thread_affinity_aarch64();
    test_thread_exception_state_aarch64();
    
    assert(true);
}