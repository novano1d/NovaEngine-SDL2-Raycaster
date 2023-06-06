#include <SDL2/SDL.h>
#include "sdlgame.hpp"
#include <iostream>
#include <map>

//This file is for testing the functionality of the library

//Global def
#define SCREEN_WIDTH 1280 
#define SCREEN_HEIGHT 720
GridGame* game;
SDL_Renderer* renderer = nullptr;
SDL_Window* window = nullptr;
Map* myMap = new Map({{1, 1, 1, 1, 1}, 
                      {1, 0, 0, 0, 1},
                      {1, 0, 0, 0, 1},
                      {1, 0, 0, 0, 1},
                      {1, 1, 1, 1, 1}});

const int FOV = 66 / 2; // FOV is first number

void playLoop()
{
    // game->clrScreen(255, 0, 0, 255);
    // game->mapGrid({0, 0, 0, 0});
    // game->drawGrid(myMap->xSize(), myMap->ySize(), {255, 255, 255, 255});
    // SDL_RenderPresent(renderer);
    SDL_SetRenderDrawColor(renderer, 74, 74, 74, 255);
    SDL_RenderClear(renderer);
    for (int i = 0; i < SCREEN_WIDTH; i++)
    {
        double scanDir = 2*i/(double)SCREEN_WIDTH - 1;
        CollisionEvent collision = game->ddaRaycast(game->getPlayerPos(), game->getAngle() + FOV * scanDir);
        int lineHeight = (int)(SCREEN_HEIGHT / (collision.perpWallDist));
        int drawStart = -lineHeight / 2 + SCREEN_HEIGHT / 2;
        if (drawStart < 0) drawStart = 0;
        int drawEnd = lineHeight / 2 + SCREEN_HEIGHT / 2;
        if (drawEnd > SCREEN_HEIGHT) drawEnd = SCREEN_HEIGHT; 
        Uint8 r = (collision.sideHit) ? 255 : 175;
        SDL_SetRenderDrawColor(renderer, r, 0, 0, 255);
        SDL_RenderDrawLine(renderer, i, drawStart, i, drawEnd);
    }
    SDL_RenderPresent(renderer);
}

void eventHandler(SDL_Event event)
{
    if (event.type == SDL_KEYDOWN)
    {
        switch (event.key.keysym.sym)
        {
        case SDLK_w:
            game->setPlayerPos({game->getPlayerPos().x + game->getMoveSpeed() * cos(game->getAngle()*M_PI/180), game->getPlayerPos().y + game->getMoveSpeed() * sin(game->getAngle()*M_PI/180)});
            break;
        case SDLK_s:
            game->setPlayerPos({game->getPlayerPos().x - game->getMoveSpeed() * cos(game->getAngle()*M_PI/180), game->getPlayerPos().y - game->getMoveSpeed() * sin(game->getAngle()*M_PI/180)});
            break;
        case SDLK_d:
            game->setAngle(game->getAngle() + game->getRotSpeed());
            break;
        case SDLK_a:
            game->setAngle(game->getAngle() - game->getRotSpeed());
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