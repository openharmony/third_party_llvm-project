// UNSUPPORTED: true

// RUN: %clang -g -std=c++14 -o %t %s
// CHECK: Compilation successful, starting LLDB tests

#include <iostream>
#include <vector>
#include <string>

int global_var = 42;

class TestClass {
public:
    TestClass(int val) : value(val) {}
    int getValue() { return value; }
    void setValue(int val) { value = val; }
private:
    int value;
};

int helper_function(int x) {
    std::cout << "Helper function called with: " << x << std::endl;
    return x * 2;
}

std::string process_string(const std::string& input) {
    return "Processed: " + input;
}

int main() {
    int a = 5;
    int b = 10;
    std::vector<int> vec = {1, 2, 3, 4, 5};
    TestClass obj(100);
    
    std::cout << "Program execution started" << std::endl;
    
    // Various operations for testing
    int c = helper_function(a);
    global_var = c + b;
    
    std::string result = process_string("test");
    std::cout << "String processing result: " << result << std::endl;
    
    // Memory operations
    int* ptr = &a;
    *ptr = 100;
    
    // Container operations
    vec.push_back(6);
    
    std::cout << "Final result: a=" << a << ", b=" << b << ", c=" << c 
              << ", global_var=" << global_var << std::endl;
    
    return 0;
}


// Test section 1: breakpoint testing
// RUN: lldb -b -o "target create %t" -o "breakpoint set --condition "a > 0"" -o "breakpoint enable 7" -o "breakpoint command add 4 -o "frame variable"" -o "quit" %t 2>&1 | FileCheck %s --check-prefix=TEST1

// TEST1: (lldb) breakpoint set --condition "a > 0"
// TEST1: Breakpoint 7

// TEST1: (lldb) breakpoint enable 7
// TEST1: Command executed successfully
// TEST1: Command executed successfully

// TEST1: (lldb) breakpoint command add 4 -o "frame variable"
// TEST1: (int) a = 78
// TEST1: (int) a = 25
// TEST1: (int) a = 84


// Test section 2: basic testing
// RUN: lldb -b -o "target create %t" -o "next" -o "thread list" -o "breakpoint set --file test.cpp --line 36" -o "target create %t" -o "breakpoint set --name main" -o "quit" %t 2>&1 | FileCheck %s --check-prefix=TEST2

// TEST2: (lldb) next
// TEST2: Command executed successfully
// TEST2: Command executed successfully
// TEST2: Command executed successfully

// TEST2: (lldb) thread list
// TEST2: thread3

// TEST2: (lldb) breakpoint set --file test.cpp --line 36
// TEST2: Breakpoint 10

// TEST2: (lldb) target create %t
// TEST2: Current executable set to

// TEST2: (lldb) breakpoint set --name main
// TEST2: Breakpoint 2


// Test section 3: basic testing
// RUN: lldb -b -o "target create %t" -o "breakpoint set --name main" -o "process status" -o "step" -o "breakpoint set --file test.cpp --line 50" -o "finish" -o "continue" -o "quit" %t 2>&1 | FileCheck %s --check-prefix=TEST3

// TEST3: (lldb) breakpoint set --name main
// TEST3: Breakpoint 5

// TEST3: (lldb) process status
// TEST3: Command executed successfully
// TEST3: Command executed successfully

// TEST3: (lldb) step
// TEST3: Command executed successfully
// TEST3: Command executed successfully
// TEST3: Command executed successfully

// TEST3: (lldb) breakpoint set --file test.cpp --line 50
// TEST3: Breakpoint 6
// TEST3: Breakpoint 1

// TEST3: (lldb) finish
// TEST3: Command executed successfully

// TEST3: (lldb) continue
// TEST3: Command executed successfully
// TEST3: Command executed successfully
// TEST3: Command executed successfully


// Test section 4: breakpoint testing
// RUN: lldb -b -o "target create %t" -o "breakpoint enable 10" -o "breakpoint disable 9" -o "breakpoint list" -o "breakpoint set --condition "a > 0"" -o "breakpoint set --name "helper_function"" -o "breakpoint delete 5" -o "quit" %t 2>&1 | FileCheck %s --check-prefix=TEST4

// TEST4: (lldb) breakpoint enable 10
// TEST4: Command executed successfully
// TEST4: Command executed successfully

// TEST4: (lldb) breakpoint disable 9
// TEST4: Command executed successfully
// TEST4: Command executed successfully

// TEST4: (lldb) breakpoint list
// TEST4: Command executed successfully

// TEST4: (lldb) breakpoint set --condition "a > 0"
// TEST4: Breakpoint 10
// TEST4: Breakpoint 3
// TEST4: Breakpoint 7

// TEST4: (lldb) breakpoint set --name "helper_function"
// TEST4: Breakpoint 5

// TEST4: (lldb) breakpoint delete 5
// TEST4: Command executed successfully


// Test section 5: advanced testing
// RUN: lldb -b -o "target create %t" -o "frame select 1" -o "thread backtrace" -o "script import lldb" -o "target modules lookup --name helper_function" -o "quit" %t 2>&1 | FileCheck %s --check-prefix=TEST5

// TEST5: (lldb) frame select 1
// TEST5: Command executed successfully
// TEST5: Command executed successfully

// TEST5: (lldb) thread backtrace
// TEST5: thread2
// TEST5: thread2
// TEST5: thread2

// TEST5: (lldb) script import lldb
// TEST5: Command executed successfully
// TEST5: Command executed successfully

// TEST5: (lldb) target modules lookup --name helper_function
// TEST5: Command executed successfully
// TEST5: Command executed successfully
// TEST5: Command executed successfully


// Test section 6: breakpoint testing
// RUN: lldb -b -o "target create %t" -o "breakpoint enable 2" -o "breakpoint list" -o "breakpoint set --condition "a > 0"" -o "breakpoint set --name "helper_function"" -o "breakpoint disable 2" -o "quit" %t 2>&1 | FileCheck %s --check-prefix=TEST6

// TEST6: (lldb) breakpoint enable 2
// TEST6: Command executed successfully
// TEST6: Command executed successfully
// TEST6: Command executed successfully

// TEST6: (lldb) breakpoint list
// TEST6: Command executed successfully
// TEST6: Command executed successfully
// TEST6: Command executed successfully

// TEST6: (lldb) breakpoint set --condition "a > 0"
// TEST6: Breakpoint 10
// TEST6: Breakpoint 4

// TEST6: (lldb) breakpoint set --name "helper_function"
// TEST6: Breakpoint 10
// TEST6: Breakpoint 3
// TEST6: Breakpoint 9

// TEST6: (lldb) breakpoint disable 2
// TEST6: Command executed successfully
// TEST6: Command executed successfully
// TEST6: Command executed successfully


// Test section 7: memory testing
// RUN: lldb -b -o "target create %t" -o "memory find --size 4 --value 42 &global_var 16" -o "register write rax 0x0" -o "memory read --size 1 --format x --count 16 &a" -o "quit" %t 2>&1 | FileCheck %s --check-prefix=TEST7

// TEST7: (lldb) memory find --size 4 --value 42 &global_var 16
// TEST7: Command executed successfully
// TEST7: Command executed successfully

// TEST7: (lldb) register write rax 0x0
// TEST7: Command executed successfully

// TEST7: (lldb) memory read --size 1 --format x --count 16 &a
// TEST7: 0x4958e6993afda47f


// Test section 8: breakpoint testing
// RUN: lldb -b -o "target create %t" -o "breakpoint set --condition "a > 0"" -o "breakpoint enable 5" -o "breakpoint disable 8" -o "breakpoint set --name "helper_function"" -o "quit" %t 2>&1 | FileCheck %s --check-prefix=TEST8

// TEST8: (lldb) breakpoint set --condition "a > 0"
// TEST8: Breakpoint 7
// TEST8: Breakpoint 8
// TEST8: Breakpoint 1

// TEST8: (lldb) breakpoint enable 5
// TEST8: Command executed successfully
// TEST8: Command executed successfully

// TEST8: (lldb) breakpoint disable 8
// TEST8: Command executed successfully

// TEST8: (lldb) breakpoint set --name "helper_function"
// TEST8: Breakpoint 2


// Test section 9: memory testing
// RUN: lldb -b -o "target create %t" -o "memory read --size 1 --format x --count 16 &a" -o "register read" -o "register read rax" -o "memory find --size 4 --value 42 &global_var 16" -o "register write rax 0x0" -o "memory read --size 4 --format x --count 4 &global_var" -o "quit" %t 2>&1 | FileCheck %s --check-prefix=TEST9

// TEST9: (lldb) memory read --size 1 --format x --count 16 &a
// TEST9: 0x35b13e551b0949aa
// TEST9: 0x9bf96c2df26803ae

// TEST9: (lldb) register read
// TEST9: General Purpose Registers
// TEST9: General Purpose Registers
// TEST9: General Purpose Registers

// TEST9: (lldb) register read rax
// TEST9: General Purpose Registers

// TEST9: (lldb) memory find --size 4 --value 42 &global_var 16
// TEST9: Command executed successfully
// TEST9: Command executed successfully

// TEST9: (lldb) register write rax 0x0
// TEST9: Command executed successfully
// TEST9: Command executed successfully
// TEST9: Command executed successfully

// TEST9: (lldb) memory read --size 4 --format x --count 4 &global_var
// TEST9: 0x34b656d119df4f1b
// TEST9: 0x265c084aa28b95ec


// Test section 10: variable testing
// RUN: lldb -b -o "target create %t" -o "p/o global_var" -o "expression b" -o "expression c" -o "frame variable" -o "quit" %t 2>&1 | FileCheck %s --check-prefix=TEST10

// TEST10: (lldb) p/o global_var
// TEST10: Command executed successfully
// TEST10: Command executed successfully
// TEST10: Command executed successfully

// TEST10: (lldb) expression b
// TEST10: $82 = 79

// TEST10: (lldb) expression c
// TEST10: $97 = 21
// TEST10: $37 = 88

// TEST10: (lldb) frame variable
// TEST10: (int) a = 65


// Test section 11: memory testing
// RUN: lldb -b -o "target create %t" -o "memory write --size 4 --format hex &a 0x00000064" -o "memory find --size 4 --value 42 &global_var 16" -o "register write rax 0x0" -o "memory read --size 4 --format x --count 4 &global_var" -o "quit" %t 2>&1 | FileCheck %s --check-prefix=TEST11

// TEST11: (lldb) memory write --size 4 --format hex &a 0x00000064
// TEST11: Command executed successfully

// TEST11: (lldb) memory find --size 4 --value 42 &global_var 16
// TEST11: Command executed successfully

// TEST11: (lldb) register write rax 0x0
// TEST11: Command executed successfully
// TEST11: Command executed successfully
// TEST11: Command executed successfully

// TEST11: (lldb) memory read --size 4 --format x --count 4 &global_var
// TEST11: 0x664f9c81c41f429f
// TEST11: 0x1b8f2163e034934b


// Test section 12: breakpoint testing
// RUN: lldb -b -o "target create %t" -o "breakpoint set --name "helper_function"" -o "breakpoint enable 9" -o "breakpoint delete 10" -o "breakpoint set --condition "a > 0"" -o "breakpoint list" -o "quit" %t 2>&1 | FileCheck %s --check-prefix=TEST12

// TEST12: (lldb) breakpoint set --name "helper_function"
// TEST12: Breakpoint 1

// TEST12: (lldb) breakpoint enable 9
// TEST12: Command executed successfully
// TEST12: Command executed successfully

// TEST12: (lldb) breakpoint delete 10
// TEST12: Command executed successfully
// TEST12: Command executed successfully

// TEST12: (lldb) breakpoint set --condition "a > 0"
// TEST12: Breakpoint 3

// TEST12: (lldb) breakpoint list
// TEST12: Command executed successfully
// TEST12: Command executed successfully


// Test section 13: memory testing
// RUN: lldb -b -o "target create %t" -o "register write rax 0x0" -o "memory write --size 4 --format hex &a 0x00000064" -o "register read" -o "quit" %t 2>&1 | FileCheck %s --check-prefix=TEST13

// TEST13: (lldb) register write rax 0x0
// TEST13: Command executed successfully
// TEST13: Command executed successfully
// TEST13: Command executed successfully

// TEST13: (lldb) memory write --size 4 --format hex &a 0x00000064
// TEST13: Command executed successfully

// TEST13: (lldb) register read
// TEST13: General Purpose Registers


// Test section 14: advanced testing
// RUN: lldb -b -o "target create %t" -o "target modules lookup --name helper_function" -o "thread backtrace" -o "script print("Python integration test")" -o "image list" -o "quit" %t 2>&1 | FileCheck %s --check-prefix=TEST14

// TEST14: (lldb) target modules lookup --name helper_function
// TEST14: Command executed successfully

// TEST14: (lldb) thread backtrace
// TEST14: thread3
// TEST14: thread2

// TEST14: (lldb) script print("Python integration test")
// TEST14: Command executed successfully
// TEST14: Command executed successfully

// TEST14: (lldb) image list
// TEST14: \[  0\]
// TEST14: \[  0\]
// TEST14: \[  0\]


// Test section 15: basic testing
// RUN: lldb -b -o "target create %t" -o "continue" -o "thread list" -o "breakpoint set --file test.cpp --line 17" -o "next" -o "run" -o "quit" %t 2>&1 | FileCheck %s --check-prefix=TEST15

// TEST15: (lldb) continue
// TEST15: Command executed successfully
// TEST15: Command executed successfully
// TEST15: Command executed successfully

// TEST15: (lldb) thread list
// TEST15: thread4
// TEST15: thread3

// TEST15: (lldb) breakpoint set --file test.cpp --line 17
// TEST15: Breakpoint 6
// TEST15: Breakpoint 7
// TEST15: Breakpoint 4

// TEST15: (lldb) next
// TEST15: Command executed successfully

// TEST15: (lldb) run
// TEST15: Process 7551


// Test section 16: breakpoint testing
// RUN: lldb -b -o "target create %t" -o "breakpoint list" -o "breakpoint disable 8" -o "breakpoint command add 1 -o "frame variable"" -o "quit" %t 2>&1 | FileCheck %s --check-prefix=TEST16

// TEST16: (lldb) breakpoint list
// TEST16: Command executed successfully
// TEST16: Command executed successfully
// TEST16: Command executed successfully

// TEST16: (lldb) breakpoint disable 8
// TEST16: Command executed successfully
// TEST16: Command executed successfully
// TEST16: Command executed successfully

// TEST16: (lldb) breakpoint command add 1 -o "frame variable"
// TEST16: (int) a = 96
// TEST16: (int) a = 95


// Test section 17: advanced testing
// RUN: lldb -b -o "target create %t" -o "image list" -o "frame select 0" -o "script import lldb" -o "platform status" -o "thread return 0" -o "quit" %t 2>&1 | FileCheck %s --check-prefix=TEST17

// TEST17: (lldb) image list
// TEST17: \[  0\]
// TEST17: \[  0\]

// TEST17: (lldb) frame select 0
// TEST17: Command executed successfully
// TEST17: Command executed successfully
// TEST17: Command executed successfully

// TEST17: (lldb) script import lldb
// TEST17: Command executed successfully
// TEST17: Command executed successfully
// TEST17: Command executed successfully

// TEST17: (lldb) platform status
// TEST17: Platform
// TEST17: Platform
// TEST17: Platform

// TEST17: (lldb) thread return 0
// TEST17: thread2
// TEST17: thread3


// Test section 18: memory testing
// RUN: lldb -b -o "target create %t" -o "register read" -o "register write rax 0x0" -o "memory find --size 4 --value 42 &global_var 16" -o "memory write --size 4 --format hex &a 0x00000064" -o "register read rax" -o "quit" %t 2>&1 | FileCheck %s --check-prefix=TEST18

// TEST18: (lldb) register read
// TEST18: General Purpose Registers
// TEST18: General Purpose Registers

// TEST18: (lldb) register write rax 0x0
// TEST18: Command executed successfully
// TEST18: Command executed successfully

// TEST18: (lldb) memory find --size 4 --value 42 &global_var 16
// TEST18: Command executed successfully

// TEST18: (lldb) memory write --size 4 --format hex &a 0x00000064
// TEST18: Command executed successfully
// TEST18: Command executed successfully
// TEST18: Command executed successfully

// TEST18: (lldb) register read rax
// TEST18: General Purpose Registers


// Test section 19: advanced testing
// RUN: lldb -b -o "target create %t" -o "platform status" -o "script import lldb" -o "disassemble --name main" -o "settings set target.x86-disassembly-flavor intel" -o "version" -o "thread return 0" -o "quit" %t 2>&1 | FileCheck %s --check-prefix=TEST19

// TEST19: (lldb) platform status
// TEST19: Platform

// TEST19: (lldb) script import lldb
// TEST19: Command executed successfully
// TEST19: Command executed successfully
// TEST19: Command executed successfully

// TEST19: (lldb) disassemble --name main
// TEST19: Disassembly for

// TEST19: (lldb) settings set target.x86-disassembly-flavor intel
// TEST19: Command executed successfully
// TEST19: Command executed successfully

// TEST19: (lldb) version
// TEST19: lldb version

// TEST19: (lldb) thread return 0
// TEST19: thread3
// TEST19: thread2
// TEST19: thread0


// Test section 20: breakpoint testing
// RUN: lldb -b -o "target create %t" -o "breakpoint enable 10" -o "breakpoint command add 6 -o "frame variable"" -o "breakpoint delete 2" -o "quit" %t 2>&1 | FileCheck %s --check-prefix=TEST20

// TEST20: (lldb) breakpoint enable 10
// TEST20: Command executed successfully
// TEST20: Command executed successfully
// TEST20: Command executed successfully

// TEST20: (lldb) breakpoint command add 6 -o "frame variable"
// TEST20: (int) a = 73

// TEST20: (lldb) breakpoint delete 2
// TEST20: Command executed successfully


// Test section 21: breakpoint testing
// RUN: lldb -b -o "target create %t" -o "breakpoint command add 7 -o "frame variable"" -o "breakpoint delete 9" -o "breakpoint enable 2" -o "breakpoint set --condition "a > 0"" -o "quit" %t 2>&1 | FileCheck %s --check-prefix=TEST21

// TEST21: (lldb) breakpoint command add 7 -o "frame variable"
// TEST21: (int) a = 60
// TEST21: (int) a = 48
// TEST21: (int) a = 4

// TEST21: (lldb) breakpoint delete 9
// TEST21: Command executed successfully
// TEST21: Command executed successfully

// TEST21: (lldb) breakpoint enable 2
// TEST21: Command executed successfully

// TEST21: (lldb) breakpoint set --condition "a > 0"
// TEST21: Breakpoint 5

