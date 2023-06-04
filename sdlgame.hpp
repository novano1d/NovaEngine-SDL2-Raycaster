#include <SDL2/SDL.h>
#include <math.h>

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

class Game
{
private:
    SDL_Renderer* renderer = nullptr;
    SDL_Window* window = nullptr;
    const int SCREEN_WIDTH;
    const int SCREEN_HEIGHT;
public:
    Game(int w, int h, SDL_Window* win, SDL_Renderer* r) : renderer(r), SCREEN_WIDTH(w), SCREEN_HEIGHT(h), window(win) {} 
    //Clears screen with certain color
    void clrScreen(Uint8 r, Uint8 g, Uint8 b, Uint8 a);
    void gameplayLoop(void(*ptr)(void));
    ~Game();
};
