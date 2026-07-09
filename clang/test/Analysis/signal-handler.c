// RUN: %clang_analyze_cc1 -analyzer-checker=openharmony -verify %s
// expected-no-diagnostics

#define SIGINT 1
void sighandler1(int);
int printf(const char *, ...);
void signal(int, void *);

int main () {
   signal(SIGINT, sighandler1);
   return 0;
}

void sighandler1(int signum) {
   printf("in signalhandler1");
}
