#include "cpu_chip8.h"

#include <fstream>
#include <iostream>
#include <iterator>
#include <vector>

#include "common.h"
#include "image.h"

#define NEXT program_counter_ += 2
#define SKIP program_counter_ += 4

constexpr int kMaxMemory = 0xFFF;
constexpr int kMaxROMSize = kMaxMemory - 0x200;

namespace { 
void DBG(const char* str, ...) {
  #ifdef DEBUG
  va_list arglist;
  va_start(arglist, str);
  vprintf(str, arglist);
  va_end(arglist);
  #endif
}
}

CpuChip8::CpuChip8() : frame_(64, 32) { }

void CpuChip8::RunCycle() {
  // Read in the big-endian opcode word.
  current_opcode_ = memory_[program_counter_] << 8 |
    memory_[program_counter_ + 1];
  DBG("\n0x%X - 0x%X\t", program_counter_, current_opcode_);

  auto instr = instructions_.find(current_opcode_);
  if (instr != instructions_.end()) {
    instr->second();
  } else {
    throw std::runtime_error("Couldn't find instruction for opcode " +
      std::to_string(current_opcode_));
  }

  // We only alter the framebuffer if the screen is cleared
  // or if a sprite is drawn.
  if (current_opcode_ == 0x00E0 || (current_opcode_ & 0xF000) == 0xD000) {
    frame_changed_ = true;
  } else {
    frame_changed_ = false;
  }

  // Update timers (assuming 60hz cycles)
  if(delay_timer_ > 0) {
    delay_timer_--;
  }
  if(sound_timer_ > 0) {
    sound_timer_--;
    if(sound_timer_ == 0) {
      // TODO: Beep.
      std::cout << "BEEP!" << std::endl;
    }
  }  
  DbgReg();
}

void CpuChip8::Initialize() {
  current_opcode_ = 0;
  std::memset(memory_, 0, 4096);
  std::memset(v_registers_, 0, 16);
  index_register_ = 0;
  program_counter_ = 0x200; 
  delay_timer_ = 0;
  sound_timer_ = 0;
  std::memset(stack_, 0, 16);
  stack_pointer_ = 0;
  std::memset(keypad_state_, 0, 16);
  frame_changed_ = true;
  
  uint8_t chip8_fontset[80] =
  { 
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
  };
  // Load the built-in fontset into 0x050-0x0A0
  std::memcpy(memory_ + 0x50, chip8_fontset, 80);
  frame_.SetAll(0);

  BuildInstructionSet();
  std::cout << "Initialization complete." << std::endl;
}

void CpuChip8::LoadROM(const std::string& filename) {
  std::ifstream input(filename, std::ios::in | std::ios::binary);
  std::vector<uint8_t> bytes(
         (std::istreambuf_iterator<char>(input)),
         (std::istreambuf_iterator<char>()));
  if (bytes.size() > kMaxROMSize) {
    throw std::runtime_error("File size is bigger than max rom size.");
  } else if (bytes.size() <= 0) {
    throw std::runtime_error("No file or empty file.");
  }
  std::memcpy(memory_ + 0x200, bytes.data(), bytes.size());
  std::cout << std::endl << std::dec << "Loaded " << bytes.size() << " byte ROM " << filename << std::endl;
  DbgMem();
}

void CpuChip8::BuildInstructionSet() {
  instructions_.clear();
  instructions_.reserve(0xFFFF);

  instructions_[0x00E0] = [this]() { frame_.SetAll(0); DBG("CLS"); NEXT; };
  instructions_[0x00EE] = [this]() {
    program_counter_ = stack_[--stack_pointer_] + 2;  // RET
    DBG("RET -- POPPED pc=0x%X off the stack.", program_counter_);
  };

  for (int opcode = 0x1000; opcode < 0xFFFF; opcode++) {
    uint16_t nnn =  opcode & 0x0FFF;
    uint8_t kk =    opcode & 0x00FF;
    uint8_t x =     (opcode & 0x0F00) >> 8;
    uint8_t y =     (opcode & 0x00F0) >> 4;
    uint8_t n =     opcode & 0x000F;
    if ((opcode & 0xF000) == 0x1000) {
      instructions_[opcode] = GenJP(nnn);
    } else if ((opcode & 0xF000) == 0x2000) {
      instructions_[opcode] = GenCALL(nnn);
    } else if ((opcode & 0xF000) == 0x3000) {
      instructions_[opcode] = GenSE(x, kk);
    } else if ((opcode & 0xF000) == 0x4000) {
      instructions_[opcode] = GenSNE(x, kk);
    } else if ((opcode & 0xF00F) == 0x5000) {
      instructions_[opcode] = GenSEREG(x, y);
    } else if ((opcode & 0xF000) == 0x6000) {
      instructions_[opcode] = GenLDIMM(x, kk);
    } else if ((opcode & 0xF000) == 0x7000) {
      instructions_[opcode] = GenADDIMM(x, kk);
    } else if ((opcode & 0xF00F) == 0x8000) {
      instructions_[opcode] = GenLDV(x, y);
    } else if ((opcode & 0xF00F) == 0x8001) {
      instructions_[opcode] = GenOR(x, y);
    } else if ((opcode & 0xF00F) == 0x8002) {
      instructions_[opcode] = GenAND(x, y);
    } else if ((opcode & 0xF00F) == 0x8003) {
      instructions_[opcode] = GenXOR(x, y);
    } else if ((opcode & 0xF00F) == 0x8004) {
      instructions_[opcode] = GenADD(x, y);
    } else if ((opcode & 0xF00F) == 0x8005) {
      instructions_[opcode] = GenSUB(x, y);
    } else if ((opcode & 0xF00F) == 0x8006) {
      instructions_[opcode] = GenSHR(x);
    } else if ((opcode & 0xF00F) == 0x8007) {
      instructions_[opcode] = GenSUBN(x, y);
    } else if ((opcode & 0xF00F) == 0x800E) {
      instructions_[opcode] = GenSHL(x);
    } else if ((opcode & 0xF00F) == 0x9000) {
      instructions_[opcode] = GenSNEREG(x, y);
    } else if ((opcode & 0xF000) == 0xA000) {
      instructions_[opcode] = GenLDI(nnn);
    } else if ((opcode & 0xF000) == 0xB000) {
      instructions_[opcode] = GenJPREG(nnn);
    } else if ((opcode & 0xF000) == 0xC000) {
      instructions_[opcode] = GenRND(x, kk);
    } else if ((opcode & 0xF000) == 0xD000) {
      instructions_[opcode] = GenDRAW(x, y, n);
    } else if ((opcode & 0xF0FF) == 0xE09E) {
      instructions_[opcode] = GenSKEY(x);
    } else if ((opcode & 0xF0FF) == 0xE0A1) {
      instructions_[opcode] = GenSNKEY(x);
    } else if ((opcode & 0xF0FF) == 0xF007) {
      instructions_[opcode] = GenRDELAY(x);
    } else if ((opcode & 0xF0FF) == 0xF00A) {
      instructions_[opcode] = GenWAITKEY(x);
    } else if ((opcode & 0xF0FF) == 0xF015) {
      instructions_[opcode] = GenWDELAY(x);
    } else if ((opcode & 0xF0FF) == 0xF018) {
      instructions_[opcode] = GenWSOUND(x);
    } else if ((opcode & 0xF0FF) == 0xF01E) {
      instructions_[opcode] = GenADDI(x);
    } else if ((opcode & 0xF0FF) == 0xF029) {
      instructions_[opcode] = GenLDSPRITE(x);
    } else if ((opcode & 0xF0FF) == 0xF033) {
      instructions_[opcode] = GenSTBCD(x);
    } else if ((opcode & 0xF0FF) == 0xF055) {
      instructions_[opcode] = GenSTREG(x);
    } else if ((opcode & 0xF0FF) == 0xF065) {
      instructions_[opcode] = GenLDREG(x);
    }
  }
}

CpuChip8::Instruction CpuChip8::GenJP(uint16_t addr) {
  return [this, addr]() {  program_counter_ = addr; DBG("JP %d", addr); };
}
CpuChip8::Instruction CpuChip8::GenCALL(uint16_t addr) {
  return [this, addr]() {
    stack_[stack_pointer_++] = program_counter_;
    DBG("CALL 0x%X - PUSH 0x%X onto stack", addr, stack_[stack_pointer_ - 1]);
    program_counter_ = addr;
  };
}
CpuChip8::Instruction CpuChip8::GenSE(uint8_t reg, uint8_t val) {
  return [this, reg, val]() {
    DBG("SE V%d, imm:%d", reg, val);
    v_registers_[reg] == val ? SKIP : NEXT;
  };
}
CpuChip8::Instruction CpuChip8::GenSNE(uint8_t reg, uint8_t val) {
  return [this, reg, val]() {
    v_registers_[reg] != val ? SKIP : NEXT;
  };
}
CpuChip8::Instruction CpuChip8::GenSEREG(uint8_t reg_x, uint8_t reg_y) {
  return [this, reg_x, reg_y]() {
    v_registers_[reg_x] == v_registers_[reg_y] ? SKIP : NEXT;
  };
}
CpuChip8::Instruction CpuChip8::GenLDIMM(uint8_t reg, uint8_t val) {
  return [this, reg, val]() {
    v_registers_[reg] = val;
    DBG("V%d <== %X", reg, val);
    NEXT;
  };
}
CpuChip8::Instruction CpuChip8::GenADDIMM(uint8_t reg, uint8_t val) {
  return [this, reg, val]() {
    DBG("V%d <== V%d + 0x%X", reg, reg, val);
    v_registers_[reg] += val; // Note: Carry flag doesn't change here.
    NEXT;
  };
}
CpuChip8::Instruction CpuChip8::GenLDV(uint8_t reg_x, uint8_t reg_y) {
  return [this, reg_x, reg_y]() {
    v_registers_[reg_x] = v_registers_[reg_y];
    NEXT;
  };
}
CpuChip8::Instruction CpuChip8::GenOR(uint8_t reg_x, uint8_t reg_y) {
  return [this, reg_x, reg_y]() {
    v_registers_[reg_x] |= v_registers_[reg_y];
    NEXT;
  };
}
CpuChip8::Instruction CpuChip8::GenAND(uint8_t reg_x, uint8_t reg_y) {
  return [this, reg_x, reg_y]() {
    v_registers_[reg_x] &= v_registers_[reg_y];
    NEXT;
  };
}
CpuChip8::Instruction CpuChip8::GenXOR(uint8_t reg_x, uint8_t reg_y) {
  return [this, reg_x, reg_y]() {
    v_registers_[reg_x] ^= v_registers_[reg_y];
    NEXT;
  };
}
CpuChip8::Instruction CpuChip8::GenADD(uint8_t reg_x, uint8_t reg_y) {
  return [this, reg_x, reg_y]() {
    uint16_t res = v_registers_[reg_x] += v_registers_[reg_y];
    v_registers_[0xF] = res > 0xFF; // set carry
    v_registers_[reg_x] = res;
    NEXT;
  };
}
CpuChip8::Instruction CpuChip8::GenSUB(uint8_t reg_x, uint8_t reg_y) {
  return [this, reg_x, reg_y]() {
    v_registers_[0xF] = v_registers_[reg_x] > v_registers_[reg_y]; // set not borrow
    v_registers_[reg_x] -= v_registers_[reg_y];
    NEXT;
  };
}
CpuChip8::Instruction CpuChip8::GenSHR(uint8_t reg_x) {
  return [this, reg_x]() {
    v_registers_[0xF] = v_registers_[reg_x] & 1;
    v_registers_[reg_x] >>= 1;
    NEXT;
  };
}
CpuChip8::Instruction CpuChip8::GenSUBN(uint8_t reg_x, uint8_t reg_y) {
  return [this, reg_x, reg_y]() {
    v_registers_[0xF] = v_registers_[reg_y] > v_registers_[reg_x]; // set not borrow
    v_registers_[reg_x] = v_registers_[reg_y] - v_registers_[reg_x];
    NEXT;
  };
}
CpuChip8::Instruction CpuChip8::GenSHL(uint8_t reg_x) {
  return [this, reg_x]() {
    v_registers_[0xF] = v_registers_[reg_x] > 0x80;
    v_registers_[reg_x] <<= 1;
    NEXT;
  };
}
CpuChip8::Instruction CpuChip8::GenSNEREG(uint8_t reg_x, uint8_t reg_y) {
  return [this, reg_x, reg_y]() {
    v_registers_[reg_x] != v_registers_[reg_y] ? SKIP : NEXT;
  };
}
CpuChip8::Instruction CpuChip8::GenLDI(uint16_t addr) {
  return [this, addr]() {
    index_register_ = addr;
    DBG("I <== 0x%X", index_register_, addr);
    NEXT;
  };
}
CpuChip8::Instruction CpuChip8::GenJPREG(uint16_t addr) {
  return [this, addr]() {  program_counter_ = v_registers_[0] + addr; };
}
CpuChip8::Instruction CpuChip8::GenRND(uint8_t reg_x, uint8_t val) {
  return [this, reg_x, val]() {
    v_registers_[reg_x] = (rand() % 256) & val;
    NEXT;
  };
}
CpuChip8::Instruction CpuChip8::GenDRAW(uint8_t reg_x, uint8_t reg_y, uint8_t n_rows) {
  return [this, reg_x, reg_y, n_rows]() {
    uint8_t x_coord = v_registers_[reg_x];
    uint8_t y_coord = v_registers_[reg_y];
    DBG("DRAW %d rows at c,r %d,%d\t", n_rows, x_coord, y_coord);
    // Width always 8 pix (1 bpp so 1 byte)
    // Height is the 4-bit n_rows, so in total read n_rows bytes from mem[I]
    bool pixels_unset = frame_.XORSprite(x_coord, y_coord, n_rows,
      memory_ + index_register_);
    v_registers_[0xF] = pixels_unset;
    NEXT;
  };
}
CpuChip8::Instruction CpuChip8::GenSKEY(uint8_t reg) {
  return [this, reg]() {
    keypad_state_[v_registers_[reg]] ? SKIP : NEXT;
  };
}
CpuChip8::Instruction CpuChip8::GenSNKEY(uint8_t reg) {
  return [this, reg]() {
    keypad_state_[v_registers_[reg]] ? NEXT : SKIP;
  };
}
CpuChip8::Instruction CpuChip8::GenRDELAY(uint8_t reg) {
  return [this, reg]() {
    v_registers_[reg] = delay_timer_;
    NEXT;
  };
}
CpuChip8::Instruction CpuChip8::GenWAITKEY(uint8_t reg) {
  return [this, reg]() {
    throw std::runtime_error("Implement waitkey!");
    NEXT;
  };
}
CpuChip8::Instruction CpuChip8::GenWDELAY(uint8_t reg) {
  return [this, reg]() {
    delay_timer_ = v_registers_[reg];
    NEXT;
  };
}
CpuChip8::Instruction CpuChip8::GenWSOUND(uint8_t reg) {
  return [this, reg]() {
    sound_timer_ = v_registers_[reg];
    NEXT;
  };
}
CpuChip8::Instruction CpuChip8::GenADDI(uint8_t reg) {
  return [this, reg]() {
    index_register_ += v_registers_[reg];
    NEXT;
  };
}
CpuChip8::Instruction CpuChip8::GenLDSPRITE(uint8_t reg) {
  return [this, reg]() {
    uint8_t digit = v_registers_[reg];
    index_register_ = 0x50 + (5 * digit);
    DBG("LDSPRITE digit %d. I <== 0x%X", digit, 0x50 + (5 * digit));
    NEXT;
  };
}
CpuChip8::Instruction CpuChip8::GenSTBCD(uint8_t reg) {
  return [this, reg]() {
    uint8_t value = v_registers_[reg];
    uint8_t val_hunds = value / 100;
    uint8_t val_tens =  (value / 10) % 10;
    uint8_t val_ones =  (value % 100) % 10;
    memory_[index_register_]     = val_hunds;
    memory_[index_register_ + 1] = val_tens;
    memory_[index_register_ + 2] = val_ones;
    DBG("SETBCD val: %d res: %d%d%d", value, val_hunds, val_tens, val_ones);
    NEXT;
  };
}
CpuChip8::Instruction CpuChip8::GenSTREG(uint8_t reg) {
  return [this, reg]() {
    for (uint8_t v = 0; v <= reg; v++) {
      memory_[index_register_ + v] = v_registers_[v];
    }
    NEXT;
  };
}
CpuChip8::Instruction CpuChip8::GenLDREG(uint8_t reg) {
  return [this, reg]() {
    DBG("LDREG ");
    for (uint8_t v = 0; v <= reg; v++) {
      DBG("(V%d <== M[%X] {%d})", v, index_register_ + v,
        memory_[index_register_ + v]);
      v_registers_[v] = memory_[index_register_ + v];
    }
    NEXT;
  };
}


void CpuChip8::DbgMem() {
  for (int i = 0; i <= 0xFFF; i += 0x10) {
    DBG("\nMEM[%03X]: ", i);
    for (int j = 0; j < 0x10; j++) {
      DBG("%#04x ", memory_[i + j]);
    }
  }
  DBG("\n");
}

void CpuChip8::DbgReg() {
  DBG("\n [ ");
  for (int i = 0; i <= 0xF; i++) {
    DBG("(V%d %d) ", i, v_registers_[i]);
  }
  DBG("(I %d) ", index_register_);
  DBG("(delay %d) ", delay_timer_);
  DBG("(sound %d) ", sound_timer_);
  DBG("] ");
}