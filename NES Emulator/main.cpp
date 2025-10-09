/*
#include <iostream>
#include "CPU.hpp"
#include "ROM.hpp"
#include "testing.hpp"
#include <vector>
#include <fstream>
*/


#include "typedef.hpp"
#include <iostream>
#include <SDL.h>
#undef main


SDL_Window* win;
SDL_Surface* surf;

int disp_init() {

    if (SDL_Init(SDL_INIT_VIDEO)) {
        return -1;
    }

    win = SDL_CreateWindow("brightNES", 100, 100, 512, 480, SDL_WINDOW_SHOWN);

    if (win == NULL) {
        SDL_Quit();
        return -1;
    }

    surf = SDL_GetWindowSurface(win);

    if (surf == NULL) {
        SDL_DestroyWindow(win);
        SDL_Quit();
        return -1;
    }

    SDL_SetWindowFullscreen(win, SDL_WINDOW_FULLSCREEN);

    return 0;
}

int disp_free() {
    SDL_DestroyWindow(win);
    return 0;
}


int main()
{
    disp_init();

    SDL_Delay(30000);

    disp_free();

    return 0;

}