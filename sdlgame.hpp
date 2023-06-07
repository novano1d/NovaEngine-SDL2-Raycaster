#include <SDL2/SDL.h>
#include <math.h>
#include <vector>
#include <iostream>
#include <unordered_set>

//Convenience
struct rgba { Uint8 r,g,b,a; } ;

//Point structure that acts as a point and doubles as a 2d vector structure
struct Point
{
    double x,y;
    Point operator*(const double& a) const
    {
        return {a*x, a*y};
    }
    Point operator+(const Point& a) const
    {
        return {a.x+x, a.y+y};
    }
};

//Game Class to clean things up a bit
class Game
{
protected:
    SDL_Renderer* renderer = nullptr;
    SDL_Window* window = nullptr;
    const int SCREEN_WIDTH;
    const int SCREEN_HEIGHT;
    double oldTime = 0;
    double time = 0;
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
    ~Game();
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
    Map(std::vector<std::vector<int>> m) : map(m) {}
    int getTileAt(int x, int y) { return map[y][x]; };
    int ySize() { return map[0].size(); };
    int xSize() { return map.size(); };
    ~Map();
private:
    //Could eventually swap int for a Tile class
    std::vector<std::vector<int>> map;
};

//Little object to tidy up raycast return
struct CollisionEvent
{
    bool hit = false; //Hit or not
    Point intersect = {-1,-1}; //point of intersection
    int sideHit = -1; //side hit
    double perpWallDist = -1; //perpendicular wall distance (from viewing plane)
};

//Specific type of game that contains a 2d map and various functions to build a game from such a 2d map
class GridGame : public Game
{
private:
    Map* map = nullptr;
    Point playerPos;
    double angle = 0;
    double moveSpeed = 1; //map units per second
    double rotSpeed = 100; //degrees per second
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
    CollisionEvent ddaRaycast(Point start, double angle);
    //Renders false 3d
    void pseudo3dRender(int FOV);
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