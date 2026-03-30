#include <chrono>
#include <thread>

int main() {
  lldb_enable_attach();

  for (int i = 0; i < 60; ++i)
    std::this_thread::sleep_for(std::chrono::seconds(1)); // break here

  return 0;
}
