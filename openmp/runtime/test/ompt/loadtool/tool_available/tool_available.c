// The OpenMP standard defines 3 ways of providing ompt_start_tool:

// 1. "statically-linking the tool’s definition of ompt_start_tool into an 
//     OpenMP application"

// RUN: %libomp-compile -DCODE -DTOOL && env OMP_TOOL_VERBOSE_INIT=stdout \
// RUN:    %libomp-run | FileCheck %s --check-prefixes CHECK,ADDRSPACE

// Note: We should compile the tool without -fopenmp as other tools developer
//      would do. Otherwise this test may pass for the wrong reasons on Darwin.

// RUN: %clang %flags -DTOOL -shared -fPIC %s -o %T/libtool.so %send-libtool

// 2. "introducing a dynamically-linked library that includes the tool’s 
//    definition of ompt_start_tool into the application’s address space"

// 2.1 Link with tool during compilation

// RUN: %libomp-compile -DCODE -L%T %no-as-needed-flag -ltool && \
// RUN:    env OMP_TOOL_VERBOSE_INIT=stdout LD_LIBRARY_PATH=$LD_LIBRARY_PATH:%tool-path %libomp-run | FileCheck %s \
// RUN:    --check-prefixes CHECK,ADDRSPACE 

// 2.2 Link with tool during compilation, but AFTER the runtime

// RUN: %libomp-compile -DCODE -L%T -lomp %no-as-needed-flag -ltool && \
// RUN:    env OMP_TOOL_VERBOSE_INIT=stdout LD_LIBRARY_PATH=$LD_LIBRARY_PATH:%tool-path %libomp-run | FileCheck %s \
// RUN:    --check-prefixes CHECK,ADDRSPACE

// 2.3 Inject tool via the dynamic loader

// RUN: %libomp-compile -DCODE && env OMP_TOOL_VERBOSE_INIT=stdout \
// RUN:    env LD_PRELOAD=%tool-path/libtool.so %libomp-run | FileCheck %s \
// RUN:    --check-prefixes CHECK,ADDRSPACE

// 3. "providing the name of a dynamically-linked library appropriate for the
//    architecture and operating system used by the application in the 
//    tool-libraries-var ICV"

// 3.1 OMP_TOOL_VERBOSE_INIT not set 

// RUN: %libomp-compile -DCODE && \
// RUN:    env OMP_TOOL_LIBRARIES=%tool-path/libtool.so %libomp-run | FileCheck %s

// 3.2 OMP_TOOL_VERBOSE_INIT disabled

// RUN: env OMP_TOOL_LIBRARIES=%tool-path/libtool.so OMP_TOOL_VERBOSE_INIT=disabled \
// RUN:    %libomp-run | FileCheck %s

// 3.3 OMP_TOOL_VERBOSE_INIT to stdout

// RUN: %libomp-compile -DCODE && env OMP_TOOL_LIBRARIES=%tool-path/libtool.so \
// RUN:    OMP_TOOL_VERBOSE_INIT=stdout %libomp-run | \
// RUN:    FileCheck %s -DPARENTPATH=%tool-path --check-prefixes CHECK,TOOLLIB

// 3.4 OMP_TOOL_VERBOSE_INIT to stderr, check merged stdout and stderr

// RUN: env OMP_TOOL_LIBRARIES=%tool-path/libtool.so OMP_TOOL_VERBOSE_INIT=stderr \
// RUN:    %libomp-run %redirect2to1 | \
// RUN:    FileCheck %s -DPARENTPATH=%tool-path --check-prefixes CHECK,TOOLLIB

// 3.5 OMP_TOOL_VERBOSE_INIT to stderr, check just stderr

// RUN: env OMP_TOOL_LIBRARIES=%tool-path/libtool.so OMP_TOOL_VERBOSE_INIT=stderr \
// RUN:    %libomp-run %redirect2to1 %redirect1tonull | \
// RUN:    FileCheck %s -DPARENTPATH=%tool-path --check-prefixes TOOLLIB

// 3.6 OMP_TOOL_VERBOSE_INIT to file "init.log"

// RUN: env OMP_TOOL_LIBRARIES=%tool-path/libtool.so OMP_TOOL_VERBOSE_INIT=%tool-path/init.log \
// RUN:    %libomp-run | FileCheck %s %recv-log && cat %T/init.log | \
// RUN:    FileCheck %s -DPARENTPATH=%tool-path --check-prefixes TOOLLIB

// REQUIRES: ompt

/*
 *  This file contains code for an OMPT shared library tool to be
 *  loaded and the code for the OpenMP executable.
 *  -DTOOL enables the code for the tool during compilation
 *  -DCODE enables the code for the executable during compilation
 */

// Check if libomp supports the callbacks for this test.
// CHECK-NOT: {{^}}0: Could not register callback

// ADDRSPACE: ----- START LOGGING OF TOOL REGISTRATION -----
// ADDRSPACE-NEXT: Search for OMP tool in current address space... Success.
// ADDRSPACE-NEXT: Tool was started and is using the OMPT interface.
// ADDRSPACE-NEXT: ----- END LOGGING OF TOOL REGISTRATION -----

// TOOLLIB: ----- START LOGGING OF TOOL REGISTRATION -----
// TOOLLIB-NEXT: Search for OMP tool in current address space... Failed.
// TOOLLIB-NEXT: Searching tool libraries...
// TOOLLIB-NEXT: OMP_TOOL_LIBRARIES = [[PARENTPATH]]/libtool.so
// TOOLLIB-NEXT: Opening [[PARENTPATH]]/libtool.so... Success.
// TOOLLIB-NEXT: Searching for ompt_start_tool in
// TOOLLIB-SAME: [[PARENTPATH]]/libtool.so... Success.
// TOOLLIB-NEXT: Tool was started and is using the OMPT interface.
// TOOLLIB-NEXT: ----- END LOGGING OF TOOL REGISTRATION -----

#ifdef CODE
#include "omp.h"

int main()
{
  #pragma omp parallel num_threads(2)
  {
  }

  // CHECK-NOT: ----- START LOGGING OF TOOL REGISTRATION -----
  // CHECK-NOT: ----- END LOGGING OF TOOL REGISTRATION -----

  // CHECK: {{^}}0: NULL_POINTER=[[NULL:.*$]]
  // CHECK: {{^}}0: ompt_event_runtime_shutdown

  return 0;
}

#endif /* CODE */

#ifdef TOOL

#include <stdio.h>
#include <omp-tools.h>

int ompt_initialize(ompt_function_lookup_t lookup, int initial_device_num,
                    ompt_data_t *tool_data) {
  printf("0: NULL_POINTER=%p\n", (void*)NULL);
  return 1; //success
}

void ompt_finalize(ompt_data_t* tool_data)
{
  printf("0: ompt_event_runtime_shutdown\n");
}

ompt_start_tool_result_t* ompt_start_tool(
  unsigned int omp_version,
  const char *runtime_version)
{
  static ompt_start_tool_result_t ompt_start_tool_result = {&ompt_initialize,&ompt_finalize, 0};
  return &ompt_start_tool_result;
}
#endif /* TOOL */
