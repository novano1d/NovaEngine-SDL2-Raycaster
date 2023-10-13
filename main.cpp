#include <SDL2/SDL.h>
#include "sdlgame.hpp"
#include <iostream>

//TODO
// texture floor map
// implement animated sprites
// directional sprites
// lighting

//Global def
// #define SCREEN_WIDTH 640 
// #define SCREEN_HEIGHT 480
#define SCREEN_WIDTH 1280 
#define SCREEN_HEIGHT 720
// #define SCREEN_WIDTH 1920 
// #define SCREEN_HEIGHT 1080
KeyHandler *keyhandler = new KeyHandler();
GridGame* game;
SDL_Renderer* renderer = nullptr;
SDL_Window* window = nullptr;
Map* myMap = new Map({{1, 1, 1, 1, 1, 1, 1, 1},
                      {1, 0, 0, 0, 0, 1, 0, 1},
                      {1, 0, 0, 0, 1, 1, 0, 1},
                      {1, 0, 0, 0, 0, 0, 0, 1},
                      {1, 0, 0, 0, 0, 0, 0, 1},
                      {1, 0, 0, 0, 0, 0, 0, 1},
                      {1, 0, 0, 0, 0, 0, 0, 1},
                      {1, 1, 1, 1, 1, 1, 1, 1}});


std::vector<std::vector<int>> floormap = {{1, 1, 1, 1, 1, 1, 1, 1},
                                          {1, 1, 1, 1, 1, 1, 1, 1},
                                          {1, 1, 1, 1, 1, 1, 1, 1},
                                          {1, 1, 1, 1, 1, 1, 1, 1},
                                          {1, 1, 1, 1, 1, 1, 1, 1},
                                          {1, 1, 1, 1, 1, 1, 1, 1},
                                          {1, 1, 1, 1, 1, 1, 1, 1},
                                          {1, 1, 1, 1, 1, 1, 1, 1}};

std::vector<std::vector<int>> ceilmap  = {{1, 1, 1, 1, 1, 1, 1, 1},
                                          {1, 1, 1, 1, 1, 1, 1, 1},
                                          {1, 1, 1, 1, 1, 1, 1, 1},
                                          {1, 1, 1, 1, 1, 1, 1, 1},
                                          {1, 1, 1, 1, 1, 1, 1, 1},
                                          {1, 1, 1, 1, 1, 1, 1, 1},
                                          {1, 1, 1, 1, 1, 1, 1, 1},
                                          {1, 1, 1, 1, 1, 1, 1, 1}};
const int FOV = 60; 

double ticktime;

//Deals with actions to be performed on certain keypresses
void handleInput()
{
    if (keyhandler->isKeyDown(SDLK_w))
        game->setPlayerPos({game->getPlayerPos().x + ticktime * game->getMoveSpeed() * cos(game->getAngle()*M_PI/180), game->getPlayerPos().y + ticktime * game->getMoveSpeed() * sin(game->getAngle()*M_PI/180)});
    if (keyhandler->isKeyDown(SDLK_s))
        game->setPlayerPos({game->getPlayerPos().x - ticktime * game->getMoveSpeed() * cos(game->getAngle()*M_PI/180), game->getPlayerPos().y - ticktime * game->getMoveSpeed()  * sin(game->getAngle()*M_PI/180)});
    if (keyhandler->isKeyDown(SDLK_d))
        game->setAngle(game->getAngle() + ticktime * game->getRotSpeed());
    if (keyhandler->isKeyDown(SDLK_a))
        game->setAngle(game->getAngle() - ticktime * game->getRotSpeed());
    if (keyhandler->isKeyDown(SDLK_LSHIFT) && game->getMoveSpeed() != 3)
    {
        game->setMoveSpeed(3);
        game->setRotSpeed(180);
    }
    else if (game->getMoveSpeed() != 1.5)
    {
        game->setMoveSpeed(1.5);
        game->setRotSpeed(110);
    }
        
}       

void playLoop()
{
    handleInput();
    ticktime = game->frameTime();
    game->pseudo3dRenderTextured(FOV);
}

void eventHandler(SDL_Event event)
{
    if (event.type == SDL_KEYDOWN) keyhandler->keyDown(event.key.keysym.sym);
    else if (event.type == SDL_KEYUP) keyhandler->keyUp(event.key.keysym.sym);
}

int main(int argc, char** argv)
{
    myMap->addSprite({4.5, 4.5, 4, 0, false, {0}, true, {5, 12, 11, 10, 9, 9 , 7, 6}});
    myMap->addSprite({3.5, 3.5, 4, 90, false, {0}, true, {5, 12, 11, 10, 9, 9 , 7, 6}});
    //myMap->addSprite({2, 2, 3});
    myMap->setFloorMap(floormap);
    myMap->setCeilingMap(ceilmap);
    TextureHandler *myTexture = new TextureHandler({"wood.jpg", "floor.jpg", "wooddoor.jpg", "globe.png", "bri.jpg", "wolf3d-guard_01.gif", "wolf3d-guard_02.gif", "wolf3d-guard_03.gif", "wolf3d-guard_04.gif", "wolf3d-guard_05.gif", "wolf3d-guard_06.png", "wolf3d-guard_07.gif", "wolf3d-guard_08.gif"});
    SDL_Init(SDL_INIT_VIDEO);
    //SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0, &window, &renderer);
    window = SDL_CreateWindow("3D!! Raycaster", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    // Create SDL renderer with target texture flag
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_TARGETTEXTURE);
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0"); // for resolution scaling
    game = new GridGame(SCREEN_WIDTH, SCREEN_HEIGHT, window, renderer);
    game->setTextureSet(myTexture);
    game->setAngle(45);
    game->setMap(myMap);
    game->setPlayerPos({1.5,1.5});
    game->setEventHandler(eventHandler);
    //std::cout << SDL_MapRGBA(SDL_AllocFormat(SDL_PIXELFORMAT_RGBA8888), 150, 0, 0, 255) << std::endl;
    game->gameplayLoop(playLoop);
    return 0;
}