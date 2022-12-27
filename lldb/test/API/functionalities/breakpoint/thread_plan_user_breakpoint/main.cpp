int func_1() { return 1; }

int func_2() {
  func_1();             // breakpoint_1
  return 1 + func_1();  // breakpoint_2
}

int main(int argc, char const *argv[]) {
  func_2();  // breakpoint_0
  return 0;
}
