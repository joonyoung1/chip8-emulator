#include "chip8.cpp"
#include <SDL2/SDL.h>
#include <iostream>

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

    Chip8 chip8;
    // chip8.loadRom("Maze (alt) [David Winter, 199x].ch8");
    chip8.loadRom("test_opcode.ch8");

    bool running = true;
    int temp;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }

        for (int i = 0; i < 10; i++) {
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
    }

    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
    SDL_Quit();

    return 0;
}
