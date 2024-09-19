#include <bitset>
#include <cstring>
#include <fstream>
#include <iostream>
#include <random>

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
        uint16_t addr;
        uint8_t x;
        uint8_t y;
        uint8_t n;
        uint8_t kk;

        std::mt19937 rng(std::random_device{}());
        std::uniform_int_distribution<int> dist(0, 255);

        opcode = memory[pc];
        opcode <<= 8;
        opcode |= memory[pc + 1];

        switch (opcode & 0xF000) {
        case 0x0000:
            switch (opcode) {
            case 0x00E0: // 00E0 - CLS
                display.reset();
                pc += 2;
                break;

            case 0x00EE: // ooEE - RET
                sp--;
                pc = stack[sp];
                pc += 2;
                break;

            default:
                pc += 2;
                break;
            }
            break;

        case 0x1000: // 1nnn - P addr
            addr = opcode & 0x0FFF;
            pc = addr;
            break;

        case 0x2000: // 2nnn - CALL addr
            addr = opcode & 0x0FFF;
            stack[sp] = pc;
            sp++;
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

        case 0x5000: // 5xy0 - SE Vx, Vy
            if ((opcode & 0x000F) == 0x0000) {
                x = (opcode & 0x0F00) >> 8;
                y = (opcode & 0x00F0) >> 4;
                pc += (V[x] == V[y]) ? 4 : 2;
            }
            break;

        case 0x6000: // 6xkk - LD Vx, byte
            x = (opcode & 0x0F00) >> 8;
            kk = opcode & 0x00FF;
            V[x] = kk;
            pc += 2;
            break;

        case 0x7000: // 7xkk - ADD Vx, byte
            x = (opcode & 0x0F00) >> 8;
            kk = opcode & 0x00FF;
            V[x] += kk;
            pc += 2;
            break;

        case 0x8000:
            switch (opcode & 0x000F) {
            case 0x0000: // 8xy0 - LD Vx, Vy
                x = (opcode & 0x0F00) >> 8;
                y = (opcode & 0x00F0) >> 4;
                V[x] = V[y];
                pc += 2;
                break;

            case 0x0001: // 8xy1 - OR Vx, Vy
                x = (opcode & 0x0F00) >> 8;
                y = (opcode & 0x00F0) >> 4;
                V[x] |= V[y];
                pc += 2;
                break;

            case 0x0002: // 8xy2 - AND Vx, Vy
                x = (opcode & 0x0F00) >> 8;
                y = (opcode & 0x00F0) >> 4;
                V[x] &= V[y];
                pc += 2;
                break;

            case 0x0003: // 8xy3 - XOR Vx, Vy
                x = (opcode & 0x0F00) >> 8;
                y = (opcode & 0x00F0) >> 4;
                V[x] ^= V[y];
                pc += 2;
                break;

            case 0x0004: // 8xy4 - ADD Vx, Vy
                V[x] += V[y];
                V[0xF] = (V[x] < V[y]) ? 1 : 0;
                pc += 2;
                break;

            case 0x0005: // 8xy5 - SUB Vx, Vy
                x = (opcode & 0x0F00) >> 8;
                y = (opcode & 0x00F0) >> 4;
                V[0xF] = (V[x] >= V[y]) ? 1 : 0;
                V[x] -= V[y];
                pc += 2;
                break;

            case 0x0006: // 8xy6 - SHR Vx {, Vy}
                x = (opcode & 0x0F00) >> 8;
                V[0xF] = V[x] & 0x1;
                V[x] >>= 1;
                pc += 2;
                break;

            case 0x0007: // 8xy7 - SUBN Vx {, Vy}
                x = (opcode & 0x0F00) >> 8;
                y = (opcode & 0x00F0) >> 4;
                V[0xF] = (V[x] <= V[y]) ? 1 : 0;
                V[x] = V[y] - V[x];
                pc += 2;
                break;

            case 0x000E: // 8xyE - SHL Vx {, Vy}
                x = (opcode & 0x0F00) >> 8;
                V[0xF] = V[x] >> 7;
                V[x] <<= 1;
                pc += 2;
                break;

            default:
                pc += 2;
                break;
            }
            break;

        case 0x9000: // 9xy0 - SNE Vx, Vy
            if ((opcode & 0xF) == 0x0) {
                x = (opcode & 0x0F00) >> 8;
                y = (opcode & 0x00F0) >> 4;
                pc += (V[x] != V[y]) ? 4 : 2;
            }
            break;

        case 0xA000: // Annn - LD I, addr
            addr = opcode & 0x0FFF;
            I = addr;
            pc += 2;
            break;

        case 0xB000: // Bnnn - JP V0, addr
            addr = opcode & 0x0FFF;
            pc = V[0x0] + addr;
            break;

        case 0xC000: // Cxkk - RND Vx, byte
            x = (opcode & 0x0F00) >> 8;
            kk = opcode & 0x00FF;
            V[x] = dist(rng) & kk;
            pc += 2;
            break;

        case 0xD000: // Dxyn - DRW Vx, Vy, nibble
            x = (opcode & 0x0F00) >> 8;
            y = (opcode & 0x00F0) >> 4;
            n = opcode & 0x000F;

            for (int spriteY = 0; spriteY < n; spriteY++) {
                uint8_t spriteByte = memory[I + spriteY];

                for (int spriteX = 0; spriteX < 8; spriteX++) {
                    if (spriteByte & (0x80 >> spriteX)) {
                        int displayX = (V[x] + spriteX) % 64;
                        int displayY = (V[y] + spriteY) % 32;
                        int i = displayY * 64 + displayX;

                        if (display[i])
                            V[0xF] = 1;
                        display[i] = display[i] ^ 1;
                    }
                }
            }
            pc += 2;
            break;

        case 0xE000:
            switch (opcode & 0x00FF) {
            case 0x009E: // Ex9E - SKP Vx
                x = (opcode & 0x0F00) >> 8;
                pc += keypad[(V[x] & 0x0F)] ? 4 : 2;
                break;

            case 0x00A1: // ExA1 - SKNP Vx
                x = (opcode & 0x0F00) >> 8;
                pc += keypad[(V[x] & 0x0F)] ? 2 : 4;
                break;

            default:
                pc += 2;
                break;
            }
            break;

        case 0xF000:
            switch (opcode & 0x00FF) {
            case 0x0007: // Fx07 - LD Vx, DT
                x = (opcode & 0x0F00) >> 8;
                V[x] = delayTimer;
                pc += 2;
                break;

            case 0x000A: // Fx0A - LD Vx, K
                for (int i = 0; i < 16; i++) {
                    if (keypad[i]) {
                        V[x] = i;
                        pc += 2;
                        break;
                    }
                }
                break;

            case 0x0015: // Fx15 - LD DT, Vx
                x = (opcode & 0x0F00) >> 8;
                delayTimer = V[x];
                pc += 2;
                break;

            case 0x0018: // Fx18 - LD ST, Vx
                x = (opcode & 0x0F00) >> 8;
                soundTimer = V[x];
                pc += 2;
                break;

            case 0x001E: // Fx1E - ADD I, Vx
                x = (opcode & 0x0F00) >> 8;
                I += V[x];
                pc += 2;
                break;

            case 0x0029: // Fx29 - LD F, Vx
                x = (opcode & 0x0F00) >> 8;
                I = V[x] * 5;
                pc += 2;
                break;

            case 0x0033: // Fx33 - LD B, Vx
                x = (opcode & 0x0F00) >> 8;
                memory[I] = V[x] / 100;
                memory[I + 1] = (V[x] / 10) % 10;
                memory[I + 2] = V[x] % 10;
                pc += 2;
                break;

            case 0x0055: // Fx55 - LD [I], Vx
                x = (opcode & 0x0F00) >> 8;
                for (int i = 0; i <= x; i++)
                    memory[I + i] = V[i];
                pc += 2;
                break;

            case 0x0065: // Fx65 - LD Vx, [I]
                x = (opcode & 0x0F00) >> 8;
                for (int i = 0; i <= x; i++)
                    V[i] = memory[I + i];
                pc += 2;
                break;

            default:
                pc += 2;
                break;
            }
        }
    }
};