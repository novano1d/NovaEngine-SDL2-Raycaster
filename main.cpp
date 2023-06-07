#include <SDL2/SDL.h>
#include "sdlgame.hpp"
#include <iostream>
#include <map> //used for keys

//This file is for testing the functionality of the library

//Global def
#define SCREEN_WIDTH 1280 
#define SCREEN_HEIGHT 720
GridGame* game;
SDL_Renderer* renderer = nullptr;
SDL_Window* window = nullptr;
Map* myMap = new Map({{1, 1, 1, 1, 1, 1}, 
                      {1, 0, 0, 0, 0, 1},
                      {1, 0, 0, 0, 0, 1},
                      {1, 0, 1, 0, 1, 1},
                      {1, 0, 1, 0, 1, 1},
                      {1, 1, 1, 1, 1, 1}});

const int FOV = 66; // FOV is first number

double ticktime;

void playLoop()
{
    ticktime = game->frameTime();
    game->pseudo3dRender(FOV);
}

void eventHandler(SDL_Event event)
{
    if (event.type == SDL_KEYDOWN)
    {
        switch (event.key.keysym.sym)
        {
        case SDLK_w:
            game->setPlayerPos({game->getPlayerPos().x + ticktime * game->getMoveSpeed() * cos(game->getAngle()*M_PI/180), game->getPlayerPos().y + ticktime * game->getMoveSpeed() * sin(game->getAngle()*M_PI/180)});
            break;
        case SDLK_s:
            game->setPlayerPos({game->getPlayerPos().x - ticktime * game->getMoveSpeed() * cos(game->getAngle()*M_PI/180), game->getPlayerPos().y - ticktime * game->getMoveSpeed()  * sin(game->getAngle()*M_PI/180)});
            break;
        case SDLK_d:
            game->setAngle(game->getAngle() + ticktime * game->getRotSpeed());
            break;
        case SDLK_a:
            game->setAngle(game->getAngle() - ticktime * game->getRotSpeed());
            break;
        default:
            break;
        }
    }
    else if (event.type == SDL_MOUSEBUTTONDOWN)
    {
    
    }
}

int main(int argc, char** argv)
{
    SDL_Init(SDL_INIT_VIDEO);
    SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0, &window, &renderer);
    game = new GridGame(SCREEN_WIDTH, SCREEN_HEIGHT, window, renderer);
    game->setAngle(45);
    game->setMap(myMap);
    game->setPlayerPos({1.5,1.5});
    game->setEventHandler(eventHandler);
    game->gameplayLoop(playLoop);
    return 0;
}