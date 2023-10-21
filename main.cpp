#include "sdlgame.hpp"
#include <iostream>

//TODO
// lighting?
// render outside area

//Global def
// #define SCREEN_WIDTH 640 
// #define SCREEN_HEIGHT 480
#define SCREEN_WIDTH nva::SCREEN_WIDTH 
#define SCREEN_HEIGHT nva::SCREEN_HEIGHT
// #define SCREEN_WIDTH 1920 
// #define SCREEN_HEIGHT 1080
KeyHandler *keyhandler = new KeyHandler();
GridGame* game;
SDL_Renderer* renderer = nullptr;
SDL_Window* window = nullptr;   
Map* myMap = new Map({{1, 1, 1, 1, 1, 1, 1, 1},
                      {1, 0, 0, 0, 0, 1, 0, 1},
                      {1, 0, 0, 0, 0, 1, 0, 1},
                      {1, 0, 0, 0, 0, 1, 0, 1},
                      {1, 0, 0, 0, 0, 0, 0, 1},
                      {1, 0, 0, 0, 0, 0, 0, 1},
                      {1, 0, 0, 0, 0, 0, 0, 1},
                      {1, 1, 1, 1, 3, 1, 1, 1}});


std::vector<std::vector<int>> floormap = {{1, 1, 1, 1, 1, 1, 1, 1},
                                          {1, 1, 1, 1, 1, 1, 1, 1},
                                          {1, 1, 1, 1, 1, 1, 1, 1},
                                          {1, 1, 1, 1, 1, 1, 1, 1},
                                          {1, 1, 1, 1, 1, 1, 1, 1},
                                          {1, 1, 1, 1, 1, 1, 1, 1},
                                          {1, 1, 1, 1, 3, 1, 1, 1},
                                          {1, 1, 1, 1, 1, 1, 1, 1}};

std::vector<std::vector<int>> ceilmap  = {{1, 1, 1, 1, 1, 1, 1, 1},
                                          {1, 1, 1, 1, 1, 1, 1, 1},
                                          {1, 1, 1, 1, 1, 1, 1, 1},
                                          {1, 1, 1, 1, 1, 1, 1, 1},
                                          {1, 1, 1, 1, 1, 1, 1, 1},
                                          {1, 1, 1, 1, 1, 1, 1, 1},
                                          {1, 1, 1, 1, 1, 1, 1, 1},
                                          {1, 1, 1, 1, 1, 1, 1, 1}};

std::vector<std::vector<Door>> doorMap  = {{{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}},
                                           {{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}},
                                           {{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}},
                                           {{0}, {0}, {0}, {0}, {0}, {0}, {1, 17, true, 1}, {0}},
                                           {{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}},
                                           {{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}},
                                           {{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}},
                                           {{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}}};

std::vector<std::vector<double>> lightMap = {{1, 1, 1, 1, 1, 1, 1, 1},
                                             {1, 1, 1, 1, 1, 1, 1, 1},
                                             {1, 1, 1, 1, 1, 1, 1, 1},
                                             {1, 1, 1, 0.5, 1, 1, 1, 1},
                                             {1, 1, 1, 1, 1, 1, 1, 1},
                                             {1, 0.7, 0.7, 0.7, 1, 1, 1, 1},
                                             {1, 0.5, 0.7, 1, 1, 1, 1, 1},
                                             {1, 0.5, 0.7, 1, 1, 1, 1, 1}};
const int FOV = 60; 
 
double ticktime;

//Deals with actions to be performed on certain keypresses
void handleInput()
{
    if (keyhandler->isKeyDown(SDLK_d))
        game->setPlayerPos({game->getPlayerPos().x - ticktime * game->getMoveSpeed() * sin(game->getAngle()*M_PI/180), game->getPlayerPos().y + ticktime * game->getMoveSpeed() * cos(game->getAngle()*M_PI/180)});
    if (keyhandler->isKeyDown(SDLK_a))
        game->setPlayerPos({game->getPlayerPos().x + ticktime * game->getMoveSpeed() * sin(game->getAngle()*M_PI/180), game->getPlayerPos().y - ticktime * game->getMoveSpeed() * cos(game->getAngle()*M_PI/180)});
    if (keyhandler->isKeyDown(SDLK_w))
        game->setPlayerPos({game->getPlayerPos().x + ticktime * game->getMoveSpeed() * cos(game->getAngle()*M_PI/180), game->getPlayerPos().y + ticktime * game->getMoveSpeed() * sin(game->getAngle()*M_PI/180)});
    if (keyhandler->isKeyDown(SDLK_s))
        game->setPlayerPos({game->getPlayerPos().x - ticktime * game->getMoveSpeed() * cos(game->getAngle()*M_PI/180), game->getPlayerPos().y - ticktime * game->getMoveSpeed()  * sin(game->getAngle()*M_PI/180)});
    if (keyhandler->isKeyDown(SDLK_RIGHT))
        game->setAngle(game->getAngle() + ticktime * game->getRotSpeed());
    if (keyhandler->isKeyDown(SDLK_LEFT))
        game->setAngle(game->getAngle() - ticktime * game->getRotSpeed());
    if (keyhandler->isKeyDown(SDLK_ESCAPE) && game->getTicks() % 17 == 1) //mod by random prime to prevent spamming the key lol
        SDL_SetRelativeMouseMode(static_cast<SDL_bool>((!SDL_GetRelativeMouseMode())));
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

double totalTime = 0; //debug var
void playLoop()
{
    handleInput();
    ticktime = game->frameTime();
    totalTime += ticktime;
    game->pseudo3dRenderTextured(FOV);
}

void eventHandler(SDL_Event event)
{
    if (event.type == SDL_KEYDOWN) keyhandler->keyDown(event.key.keysym.sym);
    if (event.type == SDL_KEYUP) keyhandler->keyUp(event.key.keysym.sym);
    if (event.type == SDL_MOUSEMOTION)
    {
        game->setAngle(game->getAngle() + event.motion.xrel * game->getMouseSens());
    }
}

int main(int argc, char** argv)
{ 
    // Sprite animSides = {2, 2, 0, 0, true, {}, true, {}, {
    //     {64, 0, 64, 1},
    //     {64, 3, 64, 2},
    //     {64, 0, 64, 1},
    //     {64, 3, 64, 2},
    //     {64, 0, 64, 1},
    //     {64, 3, 64, 2},
    //     {64, 0, 64, 1},
    //     {64, 3, 64, 2}
    //     }};
    //myMap->addSprite(animSides);
    myMap->addSprite({4.5, 4.5, 4, 0, false, {}, true, {5, 12, 11, 10, 9, 8, 7, 6}});
    myMap->addSprite({3.5, 3.5, 4, 90, false, {}, true, {5, 12, 11, 10, 9, 8, 7, 6}});
    myMap->addSprite({2, 2, 3, 0, true, {16, 13, 16, 14, 16, 15, 64, 5}});
    myMap->setFloorMap(floormap);
    myMap->setCeilingMap(ceilmap);
    myMap->setDoorMap(doorMap);
    myMap->setLightMap(lightMap);
    myMap->setSkyTexture(3);
    SDL_Init(SDL_INIT_VIDEO);
    //SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0, &window, &renderer);
    window = SDL_CreateWindow("3D!! Raycaster", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    // Create SDL renderer with target texture flag
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_TARGETTEXTURE);
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0"); // for resolution scaling
    game = new GridGame(SCREEN_WIDTH, SCREEN_HEIGHT, window, renderer);
    TextureHandler *myTexture = new TextureHandler(renderer, {"wood.jpg", "floor.jpg", "wooddoor.jpg", "globe.png", "bri.jpg", "wolf3d-guard_01.gif", "wolf3d-guard_02.gif", "wolf3d-guard_03.gif", "wolf3d-guard_04.gif", "wolf3d-guard_05.gif", "wolf3d-guard_06.png", "wolf3d-guard_07.gif", "wolf3d-guard_08.gif", "wolf-shoot_01.png", "wolf-shoot_02.png", "wolf-shoot_03.png", "texlibdoor.gif"});
    game->setTextureSet(myTexture);
    game->setAngle(0);
    game->setMap(myMap);
    game->setPlayerPos({1.5,1.5});
    game->setEventHandler(eventHandler);
    SDL_SetRelativeMouseMode(SDL_TRUE);
    //std::cout << SDL_MapRGBA(SDL_AllocFormat(SDL_PIXELFORMAT_RGBA8888), 150, 0, 0, 255) << std::endl;
    game->gameplayLoop(playLoop);
    return 0;
}