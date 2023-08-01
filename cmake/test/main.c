#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_mixer/SDL_mixer.h>

int main(int argc, char *argv[])
{
    if (SDL_Init(0) < 0) {
        SDL_Log("SDL_Init: could not initialize SDL: %s\n", SDL_GetError());
        return 1;
    }
    if (Mix_Init(0) == 0) {
        SDL_Log("Mix_Init: no sound/music loaders supported (%s)\n", Mix_GetError());
    }
    Mix_Quit();
    SDL_Quit();
    return 0;
}
