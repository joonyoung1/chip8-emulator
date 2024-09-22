cmake_minimum_required(VERSION 3.10)
project(Chip8Emulator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(SDL2 REQUIRED)

add_executable(chip8-emulator src/main.cpp src/chip8.cpp)

target_include_directories(chip8-emulator PRIVATE src ${SDL2_INCLUDE_DIRS})
target_link_libraries(chip8-emulator PRIVATE ${SDL2_LIBRARIES})
