#include <SDL2/SDL.h>
#include "sdlgame.hpp"

//This file is for testing the functionality of the library

//Global def
#define SCREEN_WIDTH 1280 
#define SCREEN_HEIGHT 720
GridGame* game;
SDL_Renderer* renderer = nullptr;
SDL_Window* window = nullptr;

void playLoop()
{
    game->clrScreen(255, 0, 0, 255);
}

void eventHandler(SDL_Event event)
{
    if (event.type == SDL_KEYDOWN)
    {
        switch (event.key.keysym.sym)
        {
        case SDLK_w:
            game->drawGrid(5, 10, {255, 255, 255, 255});
            SDL_RenderPresent(renderer);
            SDL_Delay(1000);
            break;
        default:
            break;
        }
    }
}

int main(int argc, char** argv)
{
    SDL_Init(SDL_INIT_VIDEO);
    SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0, &window, &renderer);
    game = new GridGame(SCREEN_WIDTH, SCREEN_HEIGHT, window, renderer);
    game->setEventHandler(eventHandler);
    game->gameplayLoop(playLoop);
    return 0;
}