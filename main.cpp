#include <iostream>
#include <vector>
#include <string>

#include "image.h"
#include "cpu_chip8.h"

// TODO: Emulate at 60hz. (60 cycles per second).
void Run() {
  CpuChip8 cpu;
  cpu.Initialize();
  cpu.LoadROM("/Users/river/code/chip8/roms/test_opcode.ch8");
  for (int i = 0; i < 300; ++i) {
    cpu.RunCycle();
  }
  std::cout << "Exit main() success";
}

int main() {
  try {
    Run();
  } catch (const std::exception& e) {
    std::cerr << "ERROR: " << e.what();
    return 1;
  }
}
