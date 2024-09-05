#include <bitset>

class chip8{
public:
    uint8_t memeory[4096];
    uint8_t V[16];
    uint16_t I;
    uint16_t pc;
    uint8_t sp;

    uint16_t stack[16];
    uint8_t delay_timer;
    uint8_t sound_timer;
    
    std::bitset<64 * 32> display;
    std::bitset<16> keypad;
};