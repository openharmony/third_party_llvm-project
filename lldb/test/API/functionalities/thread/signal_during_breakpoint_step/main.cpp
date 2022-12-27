// This test is intended to create a situation in which signals are received by
// a thread while it is stepping off a breakpoint. The intended behavior is to
// skip the handler and to not stop at the breakpoint we are trying to step off
// the second time, as the corresponding instruction was not executed anyway.
// If a breakpoint is hit inside the handler, the breakpoint on the line with
// the original instruction should be hit when the handler is finished.
//
// This checks stepping off breakpoints set on single instructions and function
// calls as well, to see a potential pc change when single-stepping.

#include "pseudo_barrier.h"

#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <thread>
#include <chrono>

pseudo_barrier_t g_barrier;
std::atomic<bool> g_send_signals;
int g_action = 0;
int g_signal_count = 0;
int g_cur_iter = 0;

int NUM_ITERS = 0;
int NUM_SIGNAL_ITERS = 0;

using action_t = void (*)();

void do_action_func(action_t action) {
  // Wait until all threads are running
  pseudo_barrier_wait(g_barrier);

  // Do the action
  for (g_cur_iter = 0; g_cur_iter < NUM_ITERS; g_cur_iter++) {
    g_send_signals.store(true);
    action();
  }
}

void step_in_helper() {
  g_action++; // step-in-func line
}

void step_in_func() {
  step_in_helper(); // step-in-func breakpoint
}

void do_something() { g_action++; }

void step_over_func() {
  do_something(); // step-over-func breakpoint
  g_action++;     // step-over-func line
}

void step_over() {
  g_action++; // step-over breakpoint
  g_action++; // step-over line
}

void step_out_func_helper() {
  do_something(); // step-out-func breakpoint
}

void step_out_helper() {
  g_action++; // step-out breakpoint
}

void step_out_func() {
  step_out_func_helper();
  g_action++; // [step-out-func line probably]
}

void step_out() {
  step_out_helper();
  g_action++; // [step-out line probably]
}

void step_until() {
  g_action++; // step-until breakpoint
  g_action++; // step-until line
}

void step_until_func() {
  do_something(); // step-until-func breakpoint
  g_action++;     // step-until-func line
}

void signal_handler(int sig) {
  if (sig == SIGRTMIN)
    g_signal_count += 1; // Break here in signal handler
}

/// Register a simple function to handle signal
void register_signal_handler(int signal, void (*handler)(int)) {
  sigset_t empty_sigset;
  sigemptyset(&empty_sigset);

  struct sigaction action;
  action.sa_sigaction = 0;
  action.sa_mask = empty_sigset;
  action.sa_flags = 0;
  action.sa_handler = handler;
  sigaction(signal, &action, 0);
}

int dotest() {
  action_t actions[] = {
    step_in_func,
    step_over,
    step_over_func,
    step_out,
    step_out_func,
    step_until,
    step_until_func
  };

  int action_idx = 0;

  register_signal_handler(SIGRTMIN,
                          signal_handler); // Break here and adjust
                                           // NUM_ITERS and action_idx

  // Don't let either thread do anything until they're both ready.
  pseudo_barrier_init(g_barrier, 2);

  pthread_t pid = pthread_self();
  std::thread signaller([pid]() {
    // Wait until all threads are running
    pseudo_barrier_wait(g_barrier);

    // Send user-defined signals to the current thread
    for (int i = 0; i < NUM_ITERS; i++) {
      // Wait until the next action iteration cycle
      while (!g_send_signals.exchange(false))
        std::this_thread::sleep_for(std::chrono::microseconds(1000));

      // Send some signals
      for (int i = 0; i < NUM_SIGNAL_ITERS; i++) {
        pthread_kill(pid, SIGRTMIN);
        std::this_thread::sleep_for(std::chrono::microseconds(1000));
      }
    }
  });

  do_action_func(actions[action_idx]);
  return 0; // Break here to not fall out
}

int main() {
  return dotest();
}
