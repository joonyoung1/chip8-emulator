#include "chip8.cpp"
#include <SDL2/SDL.h>
#include <chrono>
#include <iostream>
#include <map>
#include <thread>

std::map<SDL_Keycode, int> keyMap;

void initializeKeyMap() {
    keyMap[SDLK_1] = 0x1;
    keyMap[SDLK_2] = 0x2;
    keyMap[SDLK_3] = 0x3;
    keyMap[SDLK_4] = 0xC;
    keyMap[SDLK_q] = 0x4;
    keyMap[SDLK_w] = 0x5;
    keyMap[SDLK_e] = 0x6;
    keyMap[SDLK_r] = 0xD;
    keyMap[SDLK_a] = 0x7;
    keyMap[SDLK_s] = 0x8;
    keyMap[SDLK_d] = 0x9;
    keyMap[SDLK_f] = 0xE;
    keyMap[SDLK_z] = 0xA;
    keyMap[SDLK_x] = 0x0;
    keyMap[SDLK_c] = 0xB;
    keyMap[SDLK_v] = 0xF;
}

const int DISPLAY_WIDTH = 64;
const int DISPLAY_HEIGHT = 32;
const int WINDOW_SCALE = 10;

bool initializeSDL(SDL_Window*& window, SDL_Renderer*& renderer) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return false;
    }

    window =
        SDL_CreateWindow("Chip-8 Emulator", SDL_WINDOWPOS_UNDEFINED,
                         SDL_WINDOWPOS_UNDEFINED, DISPLAY_WIDTH * WINDOW_SCALE,
                         DISPLAY_HEIGHT * WINDOW_SCALE, SDL_WINDOW_SHOWN);
    if (window == nullptr) {
        std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return false;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == nullptr) {
        std::cerr << "SDL_CreateRenderer Error: " << SDL_GetError()
                  << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return false;
    }

    return true;
}

int main(int argc, char* argv[]) {
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;

    if (!initializeSDL(window, renderer))
        return 1;
    initializeKeyMap();

    Chip8 chip8;
    // chip8.loadRom("Maze (alt) [David Winter, 199x].ch8");
    // chip8.loadRom("15 Puzzle [Roger Ivie].ch8");
    // chip8.loadRom("Animal Race [Brian Astle].ch8");
    chip8.loadRom("BRIX.ch8");
    // chip8.loadRom("INVADERS.ch8");
    // chip8.loadRom("test_opcode.ch8");

    bool running = true;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            } else if (event.type == SDL_KEYDOWN) {
                auto it = keyMap.find(event.key.keysym.sym);
                if (it != keyMap.end()) {
                    chip8.keypad[it->second] = true;
                    std::cout << it->second << " pressed!" << std::endl;
                }
            } else if (event.type == SDL_KEYUP) {
                auto it = keyMap.find(event.key.keysym.sym);
                if (it != keyMap.end()) {
                    chip8.keypad[it->second] = false;
                }
            }
        }

        for (int i = 0; i < 1; i++) {
            chip8.run();
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        for (int y = 0; y < DISPLAY_HEIGHT; y++) {
            for (int x = 0; x < DISPLAY_WIDTH; x++) {
                if (chip8.display[y * DISPLAY_WIDTH + x]) {
                    SDL_Rect pixelRect;
                    pixelRect.x = x * WINDOW_SCALE;
                    pixelRect.y = y * WINDOW_SCALE;
                    pixelRect.w = WINDOW_SCALE;
                    pixelRect.h = WINDOW_SCALE;
                    SDL_RenderFillRect(renderer, &pixelRect);
                }
            }
        }
        SDL_RenderPresent(renderer);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
    SDL_Quit();

    return 0;
}
