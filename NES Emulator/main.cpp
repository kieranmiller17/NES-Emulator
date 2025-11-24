


#include "typedef.hpp"
#include <iostream>
#include <fstream>
#include <string>

#include <SDL.h>
#include <stdio.h>

#include "CPU.hpp"
#include "ROM.hpp"
#include "PPU.hpp"
#include "screen.hpp"

int main(int argc, char** argv) {

    std::string romPath;
    std::cout <<  "Enter the ROM file path to start playing:\n";

    std::getline(std::cin, romPath);

    std::ifstream file(romPath);

    if (!file) {
        std::cout << "Game ROM not found!";
        return 1;
    }

    SDL_Event event;
    bool is_running = true;

    screen::init_screen();

    CPU cpu {};
    cpu.load(romPath);
    cpu.reset();

    while (is_running) {

        cpu.execute();
        
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_KEYDOWN:
                cpu.bus.controller.pressButton(event.key.keysym.sym, 1);
                break;
            case SDL_KEYUP:
                cpu.bus.controller.pressButton(event.key.keysym.sym, 0);
                break;
            case SDL_QUIT:
                is_running = false;
                break;
            default:
                break;
            }
        }
    }

    screen::destroy_screen;

    return 0;
    

}