#include <bitset>
#include <cstring>
#include <fstream>
#include <iostream>

class Chip8 {
public:
    uint8_t memory[4096];
    uint8_t V[16];
    uint16_t I;
    uint16_t pc;
    uint8_t sp;

    uint16_t stack[16];
    uint8_t delay_timer;
    uint8_t sound_timer;

    std::bitset<64 * 32> display;
    std::bitset<16> keypad;

    Chip8() {
        initialize();
    }

    void initialize() {
        memset(memory, 0, sizeof(memory));
        memset(V, 0, sizeof(V));
        memset(stack, 0, sizeof(stack));

        display.reset();
        keypad.reset();

        I = 0;
        pc = 0x200;
        sp = 0;
        delay_timer = 0;
        sound_timer = 0;
    }

    void loadRom(std::string filename) {
        std::ifstream file(filename, std::ios::binary | std::ios::ate);

        if (!file.is_open())
            std::cerr << "Failed to open Rom file: " << filename << std::endl;

        std::streampos size = file.tellg();
        file.seekg(0, std::ios::beg);

        file.read(reinterpret_cast<char*>(this->memory + 0x200), size);
        file.close();
    }
};

int main() {
    Chip8 chip8 = Chip8();
    chip8.loadRom("Maze (alt) [David Winter, 199x].ch8");

    return 0;
}