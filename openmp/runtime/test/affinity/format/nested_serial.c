// RUN: %libomp-compile && env OMP_DISPLAY_AFFINITY=true %platform-flag %libomp-run | %python %S/check.py -c 'CHECK' %s

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include "omp_testsuite.h"

int main(int argc, char** argv) {
  omp_set_affinity_format("TESTER: tl:%L at:%a tn:%n nt:%N");
  omp_set_nested(1);
  #pragma omp parallel num_threads(1)
  {
    go_parallel_nthreads(1);
    go_parallel_nthreads(1);
    #pragma omp parallel num_threads(1)
    {
      go_parallel_nthreads(1);
    }
    go_parallel_nthreads(1);
  }
  go_parallel_nthreads(1);
  go_parallel_nthreads(1);
  return get_exit_value();
}

// CHECK: num_threads=1 TESTER: tl:1 at:0 tn:0 nt:1
// CHECK: num_threads=1 TESTER: tl:2 at:0 tn:0 nt:1
// CHECK: num_threads=1 TESTER: tl:3 at:0 tn:0 nt:1
// CHECK: num_threads=1 TESTER: tl:2 at:0 tn:0 nt:1
// CHECK: num_threads=1 TESTER: tl:1 at:0 tn:0 nt:1
