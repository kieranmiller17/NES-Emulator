
#include "typedef.hpp"
#include <iostream>

#include <SDL.h>
#include <stdio.h>

#include "screen.hpp"



static SDL_Window* window = NULL;
static SDL_Surface* surface = NULL;


int screen::init_screen() {

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
    }
    else
    {
        window = SDL_CreateWindow("NES Emulator", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 512, 480, SDL_WINDOW_SHOWN);
    }


    if (window == NULL)
    {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
    }
    else
    {
        surface = SDL_GetWindowSurface(window);
        SDL_UpdateWindowSurface(window);
    }

    return 0;
}


void screen::setColour(u32 x, u32 y, u8 r, u8 g, u8 b) {

    Uint32 colour = SDL_MapRGB(surface->format, r, g, b);

    ((Uint32*)surface->pixels)[(y*surface->w) + x] = colour;
    ((Uint32*)surface->pixels)[((y+1)*surface->w) + x] = colour;
    ((Uint32*)surface->pixels)[((y+1)*surface->w) + x + 1] = colour;
    ((Uint32*)surface->pixels)[(y*surface->w) + x + 1] = colour;
}

void screen::updateScreen() {
    SDL_UpdateWindowSurface(window);
}


void screen::destroy_screen() {

    SDL_DestroyWindow(window);
    SDL_Quit();
}


