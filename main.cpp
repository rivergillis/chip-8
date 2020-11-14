#include <iostream>
#include <vector>
#include <string>

#include "image.h"
#include "cpu_chip8.h"

// TODO: Emulate at 60hz. (60 cycles per second).
void Run() {
  CpuChip8 cpu;
  cpu.Initialize();
  cpu.LoadROM("/Users/river/code/chip8/roms/ibm_logo.ch8");
  for (int i = 0; i < 100; ++i) {
    cpu.RunCycle();
  }
}

int main() {
  try {
    Run();
  } catch (const std::exception& e) {
    std::cerr << "ERROR: " << e.what();
    return 1;
  }
}
