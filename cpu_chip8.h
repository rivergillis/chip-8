#ifndef C8_CPU_CHIP8_H_
#define C8_CPU_CHIP8_H_

#include <unordered_map>
#include <atomic>
#include <thread>
#include <functional>

#include "common.h"
#include "image.h"

// Emulates the CHIP-8 CPU in a background thread
// This class is not thread-safe -- calls to Start() and Stop() should
// originate from the same thread.

class CpuChip8 {
  public:
    // Emulated number of cycles per second.
    static constexpr int kCycleSpeedHz = 480;

    struct Options {
      std::string rom_filename = "";
      // Callbacks called by the CPU worker thread.
      // Clears and fills 16-byte keypad_state_. Called once per second.
      std::function<void(uint8_t*)> set_keypad_state_callback = nullptr;
      // Produces the CPU frame. Called as produced.
      std::function<void(Image*)> produce_frame_callback = nullptr;
    };
    CpuChip8(const Options& options);

    // Begins emulation, executing kCycleSpeedHz instructions per second
    // in a background thread. Must call Stop() prior to destruction.
    void Start();

    // Waits for the current instruction to finish executing, then joins
    // the background execution thread.
    void Stop();

  private:
    // Executes cycles until running_ becomes false.
    void EmulationLoop();

    // Resets all emulation state.
    void Initialize();

    // Loads a binary file ROM into memory.
    void LoadROM(const std::string& filename);

    // Emulate the next cycle.
    void RunCycle();

    void SetKeypadState(uint8_t (&state)[16]);

    void BuildInstructionSet();

    /// Instruction set implementation generators
    using Instruction = std::function<void(void)>;
    Instruction GenJP(uint16_t addr);
    Instruction GenCALL(uint16_t addr);
    Instruction GenSE(uint8_t reg, uint8_t val);
    Instruction GenSNE(uint8_t reg, uint8_t val);
    Instruction GenSEREG(uint8_t reg_x, uint8_t reg_y);
    Instruction GenLDIMM(uint8_t reg, uint8_t val);
    Instruction GenADDIMM(uint8_t reg, uint8_t val);
    Instruction GenLDV(uint8_t reg_x, uint8_t reg_y);
    Instruction GenOR(uint8_t reg_x, uint8_t reg_y);
    Instruction GenAND(uint8_t reg_x, uint8_t reg_y);
    Instruction GenXOR(uint8_t reg_x, uint8_t reg_y);
    Instruction GenADD(uint8_t reg_x, uint8_t reg_y);
    Instruction GenSUB(uint8_t reg_x, uint8_t reg_y);
    Instruction GenSHR(uint8_t reg_x);
    Instruction GenSUBN(uint8_t reg_x, uint8_t reg_y);
    Instruction GenSHL(uint8_t reg_x);
    Instruction GenSNEREG(uint8_t reg_x, uint8_t reg_y);
    Instruction GenLDI(uint16_t addr);
    Instruction GenJPREG(uint16_t addr);
    Instruction GenRND(uint8_t reg, uint8_t val);
    Instruction GenDRAW(uint8_t reg_x, uint8_t reg_y, uint8_t n_rows);
    Instruction GenSKEY(uint8_t reg);
    Instruction GenSNKEY(uint8_t reg);
    Instruction GenRDELAY(uint8_t reg);
    Instruction GenWAITKEY(uint8_t reg);
    Instruction GenWDELAY(uint8_t reg);
    Instruction GenWSOUND(uint8_t reg);
    Instruction GenADDI(uint8_t reg);
    Instruction GenLDSPRITE(uint8_t reg);
    Instruction GenSTBCD(uint8_t reg);
    Instruction GenSTREG(uint8_t reg);
    Instruction GenLDREG(uint8_t reg);

    void DbgMem();
    void DbgReg();

    const Options options_;

    uint16_t current_opcode_;

    // Map of opcodes to instructions. Built in Initialize().
    // Each instruction is a fn pointer and two bytes. For 0xFFFF
    // instructions this should be ~640kb, which should fit nicely in cache.
    std::unordered_map<uint16_t, std::function<void(void)>> instructions_;

    // Memory map:
    // 0x000-0x1FF - Chip 8 interpreter (contains font set in emu)
    // 0x050-0x0A0 - Used for the built in 4x5 pixel font set (0-F)
    // 0x200-0xFFF - Program ROM and work RAM
    uint8_t memory_[4096]; // 4K

    // 15 8-bit general purpose registers named V0,V1 up to VE.
    // The 16th register is used for the ‘carry flag’.
    uint8_t v_registers_[16];

    // Both range 0x000 to 0xFFF (12-bit)
    uint16_t index_register_;
    uint16_t program_counter_;

    // Count down to 0 at 60hz when set.
    uint8_t delay_timer_;
    uint8_t sound_timer_;

    uint16_t stack_[16];
    // Points to the next empty spot.
    uint16_t stack_pointer_;

    // 0 when not pressed.
    uint8_t keypad_state_[16];

    // Current working frame.
    // 64x32 image. Each pixel either full-color or no-color.
    // Drawing is done in XOR mode and if a pixel is turned off as a result of
    // drawing, the VF register is set.
    Image frame_;
    bool frame_changed_;

    // Background thread that performs emulation.
    std::thread cpu_thread_;
    // Set to true on Start() and false on Stop().
    std::atomic<bool> running_;
};

#endif