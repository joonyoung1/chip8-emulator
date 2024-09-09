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
    uint8_t delayTimer;
    uint8_t soundTimer;

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
        delayTimer = 0;
        soundTimer = 0;
    }

    void loadRom(std::string filename) {
        std::ifstream file(filename, std::ios::binary | std::ios::ate);

        if (!file.is_open())
            std::cerr << "Failed to open Rom file: " << filename << std::endl;

        std::streampos size = file.tellg();
        file.seekg(0, std::ios::beg);

        file.read(reinterpret_cast<char*>(memory + 0x200), size);
        file.close();
    }

    void run() {
        uint16_t opcode;
        uint16_t highNibble;
        uint16_t addr;
        uint8_t x;
        uint8_t y;
        uint8_t kk;

        for (;;) {
            opcode = memory[pc];
            opcode <<= 8;
            opcode |= memory[pc + 1];

            switch (opcode & 0xF000) {
                case 0x0000:
                    if (opcode == 0x00E0) { // 00E0 CLS
                        display.reset();
                        pc += 2;
                    } else if (opcode == 0x00EE) { // 00EE RET
                        pc = stack[sp];
                        sp--;
                        pc += 2;
                    }
                    break;

                case 0x1000: // 1nnn - P addr
                    addr = opcode & 0x0FFF;
                    pc = addr;
                    break;

                case 0x2000: // 2nnn - CALL addr
                    addr = opcode & 0x0FFF;
                    sp++;
                    stack[sp] = pc;
                    pc = addr;
                    break;

                case 0x3000: // 3xkk - SE Vx, byte
                    x = (opcode & 0x0F00) >> 8;
                    kk = opcode & 0x00FF;
                    pc += (V[x] == kk) ? 4 : 2;
                    break;

                case 0x4000: // 4xkk - SNE Vx, byte
                    x = (opcode & 0x0F00) >> 8;
                    kk = opcode & 0X00FF;
                    pc += (V[x] != kk) ? 4 : 2;
                    break;

                case 0x5000:
                    if ((opcode & 0x000F) == 0x0000) {
                        x = (opcode & 0x0F00) >> 8;
                        y = (opcode & 0x00F0) >> 4;
                        pc += (V[x] == V[y]) ? 4 : 2;
                    }
                    break;
            }
        }
    }
};

int main() {
    Chip8 chip8 = Chip8();
    chip8.loadRom("Maze (alt) [David Winter, 199x].ch8");

    return 0;
}