#include "sdlgame.hpp"

void Game::clrScreen(Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
    SDL_SetRenderDrawColor(renderer, r, g, b, a);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);
}

void Game::gameplayLoop(void(*ptr)(void)) 
{
    while (true)
    {
        ptr();
        SDL_Event event;
        while (SDL_PollEvent(&event)) 
            if (event.type == SDL_QUIT) { 
                SDL_DestroyRenderer(renderer);
                SDL_DestroyWindow(window);
                SDL_Quit();
            }
    }
}