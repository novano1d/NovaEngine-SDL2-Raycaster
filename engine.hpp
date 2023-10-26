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

#ifndef SDLGAME_HPP
#define SDLGAME_HPP
#include <SDL2/SDL.h>
#include <math.h>
#include <vector>
#include <algorithm>
#include <iostream>
#include <unordered_set>
#include <string>
#include <thread>
#include <unordered_map>
#include <SDL2/SDL_ttf.h>
#include "./src/include/SDL2/SDL_fox.h"
//Personal best resolution bc my engine performance is BAD
// #define INTERNAL_RENDER_RES_HORIZ 512
// #define INTERNAL_RENDER_RES_VERT 288

//360p
// #define INTERNAL_RENDER_RES_HORIZ 640
// #define INTERNAL_RENDER_RES_VERT 360
#define TICKS 64 //Ticks need to be increased if frame rate is going to be really high or you need to limit the main game loop to ~144hz
#define INTERNAL_RENDER_RES_HORIZ 320
#define INTERNAL_RENDER_RES_VERT 180
#define SKY 0xFFFFF
// #define INTERNAL_RENDER_RES_HORIZ 1920
// #define INTERNAL_RENDER_RES_VERT 1080


//probably should move load image into here
namespace nva
{
    template <typename T>
    inline T clamp(const T& n, const T& lower, const T& upper) {
    return (n < lower) ? lower : (n > upper) ? upper : n;
    }
    bool loadImage(std::vector<unsigned char>& image, const std::string& filename, int& x, int&y);
    const int MAX_THREADS = 1; //Attempt at multithreading not working well lol
    const double BRIGHTNESS = 10; //resolution of the brightness scale
    const int SCREEN_WIDTH = 1280;
    const int SCREEN_HEIGHT = 720;
    inline bool checkCirc(double cx, double cy, double r, double x, double y) {
        return ((x - cx) * (x - cx) + (y - cy) * (y - cy)) <= r * r;
    }
}

struct Door
{
    bool exists = false;
    int texIndex = 0;
    bool doorState = false; //transversible or not
    double doorProgress = 1; //scale for animation of how far the door is opened/closed
    bool orientation = 1; // 1 for on x 0 for on y
};

//Convenience
struct rgba { int r,g,b,a; } ;

//Point structure that acts as a point and doubles as a 2d vector structure
struct Point
{
    double x,y;
    Point operator*(const double& a) const
    {
        return {a*x, a*y};
    }
    Point operator+(const double& a) const
    {
        return {a+x, a+y};
    }
    Point operator+(const Point& a) const
    {
        return {a.x+x, a.y+y};
    }
    Point operator-(const Point& a) const
    {
        return {a.x-x, a.y-y};
    }
};

struct Sprite
{
    double x, y;
    int texIndex;
    int angle = 0;
    bool animated = false;
    std::vector<int> animIndexes; // [frametime (ticks), index, frametime(ticks), index]
    bool multiAngle = false;
    std::vector<int> angleIndexes;
    std::vector<std::vector<int>> animIndexesAngled; //each vector of reels [frametime (ticks), index, frametime(ticks), index] for each different angle
    int curAnimIndex = 0; // internal tracker of current frame if animated
    int lastSpriteTick = 0; //internal tick for determining current frame if animated
};

struct Entity
{
    Point pos;
    double radius = 0.2;
    std::string nametype;
    int HP = 100;
    int ID;
    
    
    //can add sprite information and loop through and handle all the entities on the map during the game loop
    //change and update their sprites appropriately
};
class EntityHandler
{
private:
    std::vector<Entity*> entities;
    inline static int ID = 0;
public:
    EntityHandler() {};
    EntityHandler(std::vector<Entity*> e){
        for (auto i = e.begin(); i != e.end(); i++){
            (**i).ID = ID;
            ID++;
            entities.push_back(*i); }}
    int addEntity(Entity *i) { 
        i->ID = ID;
        ID++;
        entities.push_back(i); 
        return (ID - 1);
        };
    Entity* entityAt(int i) { return entities.at(i); };
    void setEntityAt(int i, Entity *e) { entities[i] = e; };
    std::vector<Entity*>& getEntityVec() { return entities; };
    void deleteEntityByID(int i);
};

//This class will handle loading all necessary texture images
class TextureHandler
{
private:
    std::vector<std::vector<Uint32>> memPixels; //we need to load the texture RGB into memory since calculating the RGB value every fkn time kills frame rate (reduce regular function calls)
    std::pair<int, int>* textureSizes;
    int numTextures;
    std::vector<std::vector<unsigned char>> loadedTextures;
    std::vector<std::pair<int, int>> loadedTextureSizes; //width height pairs
public:
    TextureHandler(SDL_Renderer* renderer, std::vector<std::string>);
    int numOfTextures() { return loadedTextures.size(); };
    inline std::vector<unsigned char> textureAt(int i) { return loadedTextures[i]; };
    inline std::pair<int, int> widthHeightAt(int i) { return loadedTextureSizes[i]; };
    inline rgba colorAt(int textureIndex, int x, int y);
    inline std::vector<std::vector<unsigned char>>& getLoadedTextures() {return loadedTextures;};
};

//Game Class to clean things up a bit and provide a template
class Game
{
protected:
    SDL_PixelFormat* format = SDL_AllocFormat(SDL_PIXELFORMAT_RGBA8888);
    SDL_Renderer* renderer = nullptr;
    SDL_Window* window = nullptr;
    FOX_Font *font;
    const int SCREEN_WIDTH;
    const int SCREEN_HEIGHT;
    double oldTime = 0;
    double time = 0;
    int ticks = 0;
    void(*eventMethod)(SDL_Event) = nullptr;
public:
    Game(int w, int h, SDL_Window* win, SDL_Renderer* r) : renderer(r), SCREEN_WIDTH(w), SCREEN_HEIGHT(h), window(win) {} 
    //Clears screen with certain color
    void clrScreen(Uint8 r, Uint8 g, Uint8 b, Uint8 a);
    //Takes a function pointer to your gameplay loop
    void gameplayLoop(void(*ptr)(void));
    //Takes a function pointer to your event handler (must accept an SDL_Event)
    void setEventHandler(void(*ptr)(SDL_Event));
    //returns frameTime
    double frameTime();
    void setFont(FOX_Font* p) { font = p; };
    FOX_Font* getFont() { return font; };
    void setTicks(int t) { ticks = t; };
    int getTicks() { return ticks; };
};

/*
Data structure to hold 2d maps
canvas woks as such:
(0,0)x+--------->
y+
|
|
|
ˇ
*/
class Map
{
public:
    Map(std::vector<std::vector<int>> m, std::vector<Sprite> s = {}) : map(m) {}
    void addSprite(Sprite *s) { sprites.push_back(s); };
    Sprite& getSpriteAt(int i) { return *sprites[i];};
    void removeSpriteAtEnd() { sprites.pop_back(); };
    void removeSpriteAt(int i) { sprites.erase(sprites.begin() + i); };
    int getTileAt(int x, int y) { return map[y][x]; };
    int ySize() { return map[0].size(); };
    int xSize() { return map.size(); };
    std::vector<Sprite*> getSprites() { return sprites; };
    void setFloorMap(std::vector<std::vector<int>> m) { floorMap = m; };
    int getFloorTileAt(int x, int y) { return floorMap[y][x]; };
    void setCeilingMap(std::vector<std::vector<int>> m) { ceilingMap = m; };
    int getCeilingTileAt(int x, int y) { return ceilingMap[y][x]; };
    void setDoorMap(std::vector<std::vector<Door>> m) { doorMap = m; };
    Door getDoorTileAt(int x, int y) { return doorMap[y][x]; };
    void setDoorStateAt(int x, int y, Door d) { doorMap[y][x] = d; };
    void setLightMap(std::vector<std::vector<double>> d) { lightMap = d; };
    double getLightTileAt(int x, int y) { return lightMap[y][x]; };
    void setLightStateAt(int x, int y, double d) { lightMap[y][x] = d; };
    void setSkyTexture(int i) { skyTexture = i; };
    int getSkyTexture() {return skyTexture; };
    EntityHandler* getEntities() { return entitiesOnMap; };
    void setEntityHandler(EntityHandler* p) { entitiesOnMap = p; };
private:
    //Could eventually swap int for a Tile class
    int skyTexture;
    std::vector<std::vector<int>> map;
    std::vector<std::vector<int>> floorMap;
    std::vector<std::vector<int>> ceilingMap;
    std::vector<std::vector<Door>> doorMap;
    //lightmap values may need to be prebaked to improve performance (division x amount of times per frame adds up)
    std::vector<std::vector<double>> lightMap;
    std::vector<Sprite*> sprites;
    EntityHandler* entitiesOnMap = nullptr;
};

//Little object to tidy up raycast return
struct CollisionEvent
{
    int hit = 0; //Hit or not and index
    Point intersect = {-1,-1}; //point of intersection
    int sideHit = 0; //side hit
    double perpWallDist = -1; //perpendicular wall distance (from viewing plane)
    int tileData = -1; //Contains tile data for texture
    double doorProgress; //door data
};

//Specific type of game that contains a 2d map and various functions to build a game from such a 2d map
class GridGame : public Game
{
private:
    int gunIndex = 0;
    Map* map = nullptr;
    Point playerPos;
    double angle = 0;
    double moveSpeed = 1; //map units per second
    double rotSpeed = 100; //degrees per second
    double mouseSens = 0.1;
    TextureHandler* currentTextureSet = nullptr;
    SDL_Texture* textureBuffer;
    const double SKYSCALEFACTOR = 2;
public:
    GridGame(int w, int h, SDL_Window* win, SDL_Renderer* r) : Game(w, h, win, r) {}
    //sets the current map pointer
    void setMap(Map* m) { map = m; };
    //returns the current map pointer
    Map* getCurMap() { return map; };
    //draws a full screen grid
    void drawGrid(int rows, int cols, rgba color);
    //Fills rectangles in the grid where the map is not a floor (0)
    void mapGrid(rgba c);
    //Draws a filled rectangle
    void drawRect(SDL_Rect, rgba);
    //Performs a raycast from start point at angle on current map
    //Returns CollisionEvent
    inline CollisionEvent ddaRaycast(Point start, double angle);
    //Renders false 3d untextured
    void pseudo3dRender(int FOV, double wallheight=1);
    //Renders false 3d textured
    void pseudo3dRenderTextured(int FOV, double wallheight=1);
    void setPlayerPos(Point p);
    Point getPlayerPos() { return playerPos; };
    int getCellWidth() { return SCREEN_WIDTH / map->xSize(); };
    int getCellHeight() { return SCREEN_HEIGHT / map->ySize(); };
    void setMoveSpeed(double s) { moveSpeed = s; };
    void setRotSpeed(double s) { rotSpeed = s; };
    double getMoveSpeed() { return moveSpeed; };
    double getRotSpeed() { return rotSpeed; };
    double getAngle() { return angle; };
    void setAngle(double a) { angle = fmod(a, 360); }; //clamps angle to 0,360 degrees
    void setTextureSet(TextureHandler* t){ currentTextureSet = t; };
    double getMouseSens() { return mouseSens; };
    void setMouseSens(double d) { mouseSens = d; };
    void setGunIndex(int i) { gunIndex = i; };
    int getGunIndex() { return gunIndex; };
    int shoot(Point p, double a);
    ~GridGame();
};

//Handles checking what keys are currently down at the moment.
class KeyHandler
{
private:
    std::unordered_set<SDL_Keycode> keysDown;
public:
    void keyDown(SDL_Keycode key) { keysDown.insert(key); };
    void keyUp(SDL_Keycode key) { keysDown.erase(key); };
    bool isKeyDown(SDL_Keycode key) { return keysDown.count(key) > 0; };
};

//Controlls entities and the sprites on the map associated with them
class EntityController
{
private:
    std::unordered_map<int, int> IDtoIndex;
    EntityHandler* eh = nullptr;
    Map* m = nullptr;
public:
    EntityController(Map* im, EntityHandler* em) : m(im), eh(em) {};
    void Update(); //called when entities need to be updated in position
    void createEntityAndSpriteAt(Entity *e, Sprite *s, Point pos, double radius, std::string type="NULL");
    void removeEntityAndSpriteByID(int id);
    void updateEntityRelPos(int ID, double x, double y);
};



#endif