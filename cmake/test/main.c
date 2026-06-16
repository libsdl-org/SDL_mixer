#define SDL_MAIN_HANDLED
#include "SDL.h"
#include "SDL_mixer.h"
#include <stdio.h>

int main(int argc, char *argv[]) {
    SDL_SetMainReady();
    if (SDL_Init(0) < 0) {
        fprintf(stderr, "SDL_Init: could not initialize SDL: %s\n", SDL_GetError());
        return 1;
    }
    if (Mix_Init(MIX_INIT_OGG) & MIX_INIT_OGG) {
        fprintf(stderr, "Mix_Init: Ogg Vorbis supported\n");
    } else {
        fprintf(stderr, "Mix_Init: Ogg Vorbis not supported (%s)\n", Mix_GetError());
    }
    Mix_Quit();
    SDL_Quit();
    return 0;
}
