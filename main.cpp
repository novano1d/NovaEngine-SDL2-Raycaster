#include <SDL2/SDL.h>
#include "sdlgame.hpp"

#define SCREEN_WIDTH 1280 
#define SCREEN_HEIGHT 720
Game* game;

void playLoop()
{
    game->clrScreen(255, 0, 0, 255);
    SDL_Delay(100);
    game->clrScreen(0, 255, 0, 255);
    SDL_Delay(100);
    game->clrScreen(0, 0, 255, 255);
    SDL_Delay(100);
}

int main(int argc, char** argv)
{
    SDL_Renderer* renderer = nullptr;
    SDL_Window* window = nullptr;
    SDL_Init(SDL_INIT_VIDEO);
    SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0, &window, &renderer);
    game = new Game(SCREEN_WIDTH, SCREEN_HEIGHT, window, renderer);
    game->gameplayLoop(playLoop);
}