#include <bitset>
#include <random>

struct Chip8Params {
    uint16_t fontAddr = 0x0;
    bool shiftAssignsVyToVx = false;
    bool overflowOnAddI = false;
    bool autoIncrementI = false;
};

class Chip8 {
public:
    uint8_t memory[4096];
    uint8_t V[16];
    uint16_t I;
    uint16_t pc;
    uint8_t sp;

    uint16_t stack[16];
    uint8_t delayTimer;
    uint8_t soundTimer;

    std::bitset<64 * 32> display;
    std::bitset<16> keypad;

    std::mt19937 rng;
    std::uniform_int_distribution<int> dist{0, 255};

    bool drawFlag = false;
    bool soundFlag = false;

    uint16_t fontAddr;
    bool shiftAssignsVyToVx;
    bool overflowOnAddI;
    bool autoIncrementI;

    const uint8_t fontset[80] = {
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

    Chip8(const Chip8Params& chip8Params);

    void initialize();
    void loadRom(std::string filename);
    void runCycle();
    void decrementTimers();

    bool getDrawFlag();
    void setDrawFlag(bool drawFlag);
    bool getSoundFlag();
    void setSoundFlag(bool soundFlag);
};