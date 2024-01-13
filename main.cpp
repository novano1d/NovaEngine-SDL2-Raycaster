/*

Copyright © 2023 Matthew Moore

This engine is free software. You can redistribute it and/or modify it under the terms of the License below.
The Nova SDL Game Library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

This engine is licensed under a Creative Commons Attribution-ShareAlike 4.0 International License.
https://creativecommons.org/licenses/by-sa/4.0/

 You are free to:

    Share — copy and redistribute the material in any medium or format for any purpose, even commercially.
    Adapt — remix, transform, and build upon the material for any purpose, even commercially. 

 Under the following terms:

    Attribution - You must give appropriate credit, provide a link to the license, and indicate if changes were made. You may do so in any reasonable manner, but not in any way that suggests the licensor endorses you or your use.
    ShareAlike - If you remix, transform, or build upon the material, you must distribute your contributions under the same license as the original. 

*/

#include "engine.hpp"
#include "Pathfinding.hpp"
/*          TODO LIST
    *ui
    *timer factory
    *Pathfinding (A*)
        -AI
    *menu
    *loading and storing into binary
    *re org code better (sprites and entites the same)
        -Maybe static sprite entity factory? gameobject or something
*/

//Global def
#define SCREEN_WIDTH nva::SCREEN_WIDTH 
#define SCREEN_HEIGHT nva::SCREEN_HEIGHT
KeyHandler *keyhandler = new KeyHandler();
GridGame* game;
SDL_Renderer* renderer = nullptr;
SDL_Window* window = nullptr;
Pathfinder *pf = new Pathfinder();
Map* myMap = new Map({{1, 1, 1, 1, 1, 1, 1, 1},
                      {1, 0, 0, 0, 1, 0, 0, 1},
                      {1, 0, 0, 0, 1, 0, 0, 1},
                      {1, 0, 1, 0, 1, 0, 0, 1},
                      {1, 0, 0, 0, 0, 0, 0, 1},
                      {1, 0, 1, 1, 1, 0, 0, 1},
                      {1, 0, 0, 0, 1, 0, 0, 1},
                      {1, 1, 1, 1, 1, 1, 1, 1}});

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
                                           {{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}},
                                           {{0}, {0}, {0}, {0}, {1, 17, true, 1, 0, 1, 2, DOOR_CLOSED}, {0}, {0}, {0}},
                                           {{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}},
                                           {{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}},
                                           {{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}}};

std::vector<std::vector<double>> lightMap = {{.1, .1, .1, .1, .1, .1, 1, 1},
                                             {.1, .1, .1, .1, .1, 1, 1, 1},
                                             {0.3, 0.3, 0.3, 0.3, 0.3, 1, 1, 1},
                                             {0.6, 0.6, 0.6, 0.6, 0.6, 1, 1, 1},
                                             {0.6, 0.6, 0.9, 0.9, 1, 1, 1, 1},
                                             {0.6, 0.6, 0.9, 1, 1, 1, 1, 1},
                                             {0.6, 0.6, 0.9, 0.9, 1, 1, 1, 1},
                                             {1, 0.6, 0.9, .9, 1, 1, 1, 1}};


EntityHandler *mapEntities = new EntityHandler();
EntityController *entCon = new EntityController(myMap, mapEntities);

const int FOV = 105; 
 
double ticktime;
SDL_TimerID timerID;
bool canShoot = true;
Uint32 resetGun(Uint32 interval, void* name) {
    canShoot = true;
    game->setGunIndex(17);
    SDL_RemoveTimer(timerID);
    return 0;
}

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
    if (keyhandler->isKeyDown(SDLK_ESCAPE) && game->getTicks() % 17 == 0) //mod by random prime to prevent spamming the key lol
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
    if (keyhandler->isKeyDown(SDLK_LCTRL) && canShoot) 
    {
        game->setGunIndex(18);
        canShoot = false;
        int shot = game->shoot(game->getPlayerPos(), game->getAngle());
        if (shot != -1) entCon->removeEntityAndSpriteByID(shot);
        timerID = SDL_AddTimer(750, resetGun, const_cast<char*>("SDL"));
        static Sprite s = {4.5, 4.5, 4, 0, false, {}, true, {5, 12, 11, 10, 9, 8, 7, 6}, {}, 0, 0};
        static Entity e = {{4.5, 4.5}, 0.2, "TEST"};
        entCon->createEntityAndSpriteAt(&e, &s, game->getPlayerPos(), 0.2);
        if (game->getCurMap()->isDoorNeighbor(game->getPlayerPos()))
            game->getCurMap()->toggleDoorByID(1);
    }
    if (keyhandler->isKeyDown(SDLK_RCTRL)) 
    {
        try
        {
            Point location = entCon->getPosByID(0);
            Node start = { { (int)location.x, (int)location.y } };
            Node end = { {(int)game->getPlayerPos().x, (int)game->getPlayerPos().y} };
            auto test = pf->aStar(start, end);
            std::vector<Node> path;
            for (Node node : test) {
                //std::cout << node.pos.x << " " << node.pos.y << std::endl;
                path.push_back(node);
            }
            if (path.empty()) return;
            else
            {
                if (path.size() < 2) return;
                Node nextNode = path.at(1);
                //end = { { nextNode.pos.x, nextNode.pos.y } };
                Point endp = {((int)nextNode.pos.x) + 0.5, ((int)nextNode.pos.y) + 0.5};
                double angle = Pathfinder::calcAngle(location, endp) * (M_PI / 180.0);
                double xcom, ycom;
                double speed = 1;
                xcom = speed * cos(angle), ycom = speed * sin(angle); 
                entCon->updateEntityRelPos(0, xcom * ticktime, ycom * ticktime);
            }
            // Node nextNode;
            // if (test.size() >= 1) nextNode = test.at(0);
            // else nextNode = end;
            // end = { { nextNode.pos.x, nextNode.pos.y } };
            // Point endp = {(int)end.pos.x + 0.5, (int)end.pos.y + 0.5};
            // double angle = Pathfinder::calcAngle(location, endp) * (M_PI / 180.0);
            // double xcom, ycom;
            // xcom = 0.1 * cos(angle), ycom = 0.1 * sin(angle); 
            // entCon->updateEntityRelPos(0, xcom * ticktime, ycom * ticktime);
        }
        catch(const std::exception& e)
        {
            std::cout << "error\n";
        }
    }
}       

double totalTime = 0; //debug var
void playLoop()
{
    handleInput();
    ticktime = game->frameTime();
    game->getCurMap()->updateDoors(ticktime);
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
    // myMap->addSprite(&animSides);
    //need to create an object for the game that handles the sprites for all the entities
    // myMap->addSprite({4.5, 4.5, 4, 0, false, {}, true, {5, 12, 11, 10, 9, 8, 7, 6}, {}, 0, 0});
    // mapEntities->addEntity({{4.5, 4.5}, 0.2, "TEST"});
    myMap->setEntityHandler(mapEntities);
    static Sprite s = {4.5, 4.5, 4, 0, false, {}, true, {5, 12, 11, 10, 9, 8, 7, 6}, {}, 0, 0};
    static Entity e = {{4.5, 4.5}, 0.1, "TEST"};
    static Sprite s2 = {4.5, 4.5, 4, 45, false, {}, true, {5, 12, 11, 10, 9, 8, 7, 6}, {}, 0, 0};
    static Entity e2 = {{4.5, 4.5}, 0.1, "TEST"};
    entCon->createEntityAndSpriteAt(&e, &s, {2, 2}, 0.2);
    entCon->createEntityAndSpriteAt(&e2, &s2, {2.5, 2.5}, 0.2);
    //myMap->addSprite({3.5, 3.5, 4, 90, false, {}, true, {5, 12, 11, 10, 9, 8, 7, 6}});
    //myMap->addSprite({2, 2, 3, 0, true, {32, 13, 32, 14, 32, 15, 160, 5}});
    myMap->setFloorMap(floormap);
    myMap->setCeilingMap(ceilmap);
    myMap->setDoorMap(doorMap);
    myMap->setLightMap(lightMap);
    myMap->setSkyTexture(3);
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);
    TTF_Init();
    FOX_Init();
    window = SDL_CreateWindow("3D!! Raycaster", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    // Create SDL renderer with target texture flag
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_TARGETTEXTURE | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0"); // for resolution scaling
    game = new GridGame(SCREEN_WIDTH, SCREEN_HEIGHT, window, renderer);
    TextureHandler *myTexture = new TextureHandler(renderer, {"wood.jpg", "floor.jpg", "wooddoor.jpg", "globe.png", "bri.jpg", "wolf3d-guard_01.gif", "wolf3d-guard_02.gif", "wolf3d-guard_03.gif", "wolf3d-guard_04.gif", "wolf3d-guard_05.gif", "wolf3d-guard_06.png", "wolf3d-guard_07.gif", "wolf3d-guard_08.gif", "wolf-shoot_01.png", "wolf-shoot_02.png", "wolf-shoot_03.png", "texlibdoor.gif", "DESuperShotgun_f02.png", "DESuperShotgun_f03.png"});
    game->setTextureSet(myTexture);
    game->setAngle(0);
    game->setMap(myMap);
    game->setPlayerPos({1.5,1.5});
    game->setEventHandler(eventHandler);
    SDL_SetRelativeMouseMode(SDL_TRUE);
    //SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
    game->setFont(FOX_OpenFont(renderer, "./fonts/SuboleyaRegular.ttf", 25));
    game->setGunIndex(17);
    pf->setMap(myMap);
    game->gameplayLoop(playLoop);	
    TTF_Quit();
    FOX_CloseFont(game->getFont());
    FOX_Exit();
    return 0;
}