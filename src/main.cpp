#include "chip8.h"

#include <SDL2/SDL.h>
#include <atomic>
#include <chrono>
#include <iostream>
#include <map>
#include <set>
#include <thread>

const int DISPLAY_WIDTH = 64;
const int DISPLAY_HEIGHT = 32;
const int WINDOW_SCALE = 10;

const int IPS = 500;
const int FPS = 60;

const int SAMPLE_RATE = 44100;
const int AMPLITUDE = 28000;
const double BEEP_FREQUENCY = 440.0;

std::map<SDL_Keycode, int> keyMap = {
    {SDLK_1, 0x1}, {SDLK_2, 0x2}, {SDLK_3, 0x3}, {SDLK_4, 0xC},
    {SDLK_q, 0x4}, {SDLK_w, 0x5}, {SDLK_e, 0x6}, {SDLK_r, 0xD},
    {SDLK_a, 0x7}, {SDLK_s, 0x8}, {SDLK_d, 0x9}, {SDLK_f, 0xE},
    {SDLK_z, 0xA}, {SDLK_x, 0x0}, {SDLK_c, 0xB}, {SDLK_v, 0xF}};
std::atomic<bool> running{true};

void audioCallback(void* userdata, Uint8* stream, int len) {
    Sint16* buffer = reinterpret_cast<Sint16*>(stream);
    int samples = len / sizeof(Sint16);

    static double phase = 0.0;

    for (int i = 0; i < samples; ++i) {
        buffer[i] = static_cast<Sint16>(AMPLITUDE * sin(phase));
        phase += (2.0 * M_PI * BEEP_FREQUENCY) / SAMPLE_RATE;
        if (phase >= 2.0 * M_PI)
            phase -= 2.0 * M_PI;
    }
}

bool initializeSDL(SDL_Window*& window, SDL_Renderer*& renderer) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return false;
    }

    SDL_AudioSpec audioSpec;
    audioSpec.freq = SAMPLE_RATE;
    audioSpec.format = AUDIO_S16SYS;
    audioSpec.channels = 1;
    audioSpec.samples = 2048;
    audioSpec.callback = audioCallback;
    audioSpec.userdata = nullptr;

    if (SDL_OpenAudio(&audioSpec, nullptr) < 0) {
        std::cerr << "SDL_OpenAudio Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return false;
    }

    window =
        SDL_CreateWindow("Chip-8 Emulator", SDL_WINDOWPOS_UNDEFINED,
                         SDL_WINDOWPOS_UNDEFINED, DISPLAY_WIDTH * WINDOW_SCALE,
                         DISPLAY_HEIGHT * WINDOW_SCALE, SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return false;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "SDL_CreateRenderer Error: " << SDL_GetError()
                  << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return false;
    }

    return true;
}

void cpuThread(Chip8& chip8) {
    const int INTERVAL_US = 1000000 / IPS;

    while (running) {
        auto start = std::chrono::high_resolution_clock::now();
        chip8.runCycle();
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::micro> elapsed = end - start;

        int sleepTime = INTERVAL_US - static_cast<int>(elapsed.count());
        if (sleepTime > 0) {
            std::this_thread::sleep_for(std::chrono::microseconds(sleepTime));
        }
    }
}

void handleInput(Chip8& chip8, SDL_Event& event) {
    if (event.type == SDL_QUIT) {
        running = false;
    } else if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
        auto it = keyMap.find(event.key.keysym.sym);
        if (it != keyMap.end()) {
            chip8.keypad[it->second] = (event.type == SDL_KEYDOWN);
        }
    }
}

void renderDisplay(Chip8& chip8, SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    for (int y = 0; y < DISPLAY_HEIGHT; ++y) {
        for (int x = 0; x < DISPLAY_WIDTH; ++x) {
            if (chip8.display[y * DISPLAY_WIDTH + x]) {
                SDL_Rect pixelRect = {x * WINDOW_SCALE, y * WINDOW_SCALE,
                                      WINDOW_SCALE, WINDOW_SCALE};
                SDL_RenderFillRect(renderer, &pixelRect);
            }
        }
    }
    SDL_RenderPresent(renderer);
}

void mainLoop(Chip8& chip8, SDL_Renderer* renderer) {
    SDL_Event event;
    const int INTERVAL_US = 1000000 / FPS;

    while (running) {
        auto frameStart = std::chrono::high_resolution_clock::now();

        while (SDL_PollEvent(&event)) {
            handleInput(chip8, event);
        }

        chip8.decrementTimers();

        if (chip8.getDrawFlag()) {
            renderDisplay(chip8, renderer);
            chip8.setDrawFlag(false);
        }

        if (chip8.getSoundFlag()) {
            SDL_PauseAudio(0);
            chip8.setSoundFlag(false);
        } else {
            SDL_PauseAudio(1);
        }

        auto frameEnd = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::micro> frameElapsed =
            frameEnd - frameStart;

        int sleepTime = INTERVAL_US - static_cast<int>(frameElapsed.count());
        if (sleepTime > 0) {
            std::this_thread::sleep_for(std::chrono::microseconds(sleepTime));
        }
    }
}

int main(int argc, char* argv[]) {
    std::string file;
    Chip8Params chip8Params;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "-h" || arg == "--help") {
            std::cout << "Usage: ./program <file_path> [-h|--help] [-f "
                         "location] [-s] [-v]\n";
            return 0;
        } else if (arg == "-f") {
            if (i + 1 >= argc) {
                std::cerr << "Error: No valid memory address provided."
                          << std::endl;
                return 1;
            }

            std::string fontArg = argv[++i];
            int base =
                (fontArg.find_first_of("xX") != std::string::npos) ? 16 : 10;
            chip8Params.fontAddr = std::stoi(fontArg, nullptr, base);

        } else if (arg[0] == '-') {
            if (arg[1] == '-') {
                if (arg == "--shift-vs") {
                    chip8Params.shiftAssignsVyToVx = true;
                } else if (arg == "--i-overflow") {
                    chip8Params.overflowOnAddI = true;
                } else if (arg == "--increment-i") {
                    chip8Params.autoIncrementI = true;
                }
            } else {
                for (size_t j = 1; j < arg.size(); ++j) {
                    switch (arg[j]) {
                        case 's':
                            chip8Params.shiftAssignsVyToVx = true;
                            break;
                        case 'o':
                            chip8Params.overflowOnAddI = true;
                            break;
                        case 'i':
                            chip8Params.autoIncrementI = true;
                            break;
                    }
                }
            }
        } else {
            file = argv[i];
        }
    }

    if (file.empty()) {
        std::cerr << "Error: No valid rom file path provided." << std::endl;
        return 1;
    }

    Chip8 chip8 = Chip8(chip8Params);

    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;

    if (!initializeSDL(window, renderer))
        return 1;

    chip8.loadRom(file);

    std::thread cpu(cpuThread, std::ref(chip8));
    mainLoop(chip8, renderer);

    cpu.join();
    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
    SDL_Quit();

    return 0;
}
