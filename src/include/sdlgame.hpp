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
    Game(int SCREEN_WIDTH, int SCREEN_HEIGHT);
    SDL_Renderer* getRenderer() { return renderer; };
    ~Game();
};
