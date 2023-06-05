#include <SDL2/SDL.h>
#include "sdlgame.hpp"
#include <iostream>

//This file is for testing the functionality of the library

//Global def
#define SCREEN_WIDTH 1280 
#define SCREEN_HEIGHT 720
GridGame* game;
SDL_Renderer* renderer = nullptr;
SDL_Window* window = nullptr;
Map* myMap = new Map({{0, 0, 1, 1}, 
                      {0, 1, 0, 1},
                      {0, 0, 0, 1},
                      {1, 1, 1, 0}});
        
Point cameraPlane = {0, 0.66};
Point dir {-1, 0}; //direction vector
const int FOV = 66;

void playLoop()
{
    // game->clrScreen(255, 0, 0, 255);
    // game->mapGrid({0, 0, 0, 0});
    // game->drawGrid(myMap->xSize(), myMap->ySize(), {255, 255, 255, 255});
    // SDL_RenderPresent(renderer);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);
    for (int i = 0; i < SCREEN_WIDTH; i++)
    {
        double scanDir = 2*i/(double)SCREEN_WIDTH - 1;
        CollisionEvent collision = game->ddaRaycast(game->getPlayerPos(), game->getAngle() + FOV * scanDir);
        int lineHeight = (int)(SCREEN_HEIGHT / collision.perpWallDist);
        int drawStart = -lineHeight / 2 + SCREEN_HEIGHT / 2;
        if (drawStart < 0) drawStart = 0;
        int drawEnd = lineHeight / 2 + SCREEN_HEIGHT / 2;
        if (drawEnd > SCREEN_HEIGHT) drawEnd = SCREEN_HEIGHT; 
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
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
        case SDLK_0:
            break;
        default:
            break;
        }
    }
    else if (event.type == SDL_MOUSEBUTTONDOWN)
    {
        game->setPlayerPos({(event.motion.x/(double)game->getCellWidth()), (event.motion.y/(double)game->getCellHeight())});
        std::cout << "(" << game->getPlayerPos().x << ", "  << game->getPlayerPos().y << ")";
        for (int i = 0; i < 360; i++)
        {
            CollisionEvent collision = game->ddaRaycast(game->getPlayerPos(), i);
            if (collision.hit)
            {
                SDL_RenderDrawLine(renderer, event.motion.x, event.motion.y, collision.intersect.x*game->getCellWidth(), collision.intersect.y*game->getCellHeight());
            }
        }
        SDL_RenderPresent(renderer);
    }
    
}

int main(int argc, char** argv)
{
    SDL_Init(SDL_INIT_VIDEO);
    SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0, &window, &renderer);
    game = new GridGame(SCREEN_WIDTH, SCREEN_HEIGHT, window, renderer);
    game->setPlayerPos({0.5,0.5});
    game->setMap(myMap);
    game->setEventHandler(eventHandler);
    game->gameplayLoop(playLoop);
    return 0;
}