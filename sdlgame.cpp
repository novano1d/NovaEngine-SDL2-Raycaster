#include "sdlgame.hpp"

// Game class implementation

// Sets screen color
void Game::clrScreen(Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
    SDL_SetRenderDrawColor(renderer, r, g, b, a);
    SDL_RenderClear(renderer);
}

void Game::gameplayLoop(void(*ptr)(void)) 
{
    bool keepRunning = true;
    while (keepRunning)
    {
        ptr();
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT) 
            { 
                SDL_DestroyRenderer(renderer);
                SDL_DestroyWindow(window);
                SDL_Quit();
                keepRunning = false;
                break;
            }
            else if (event.type && eventMethod)
            {
                eventMethod(event);
            }
        }
    }
}

void Game::setEventHandler(void(*ptr)(SDL_Event))
{
    eventMethod = ptr;
}


//GridGame implementation

void GridGame::drawGrid(int rows, int cols, rgba c)
{
    SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
    int cellWidth = SCREEN_WIDTH / cols;
    int cellHeight = SCREEN_HEIGHT / rows;
    for (int i = 1; i <= rows; i++) SDL_RenderDrawLine(renderer, 0, i*cellHeight, SCREEN_WIDTH, i*cellHeight);
    for (int i = 1; i <= cols; i++) SDL_RenderDrawLine(renderer, i*cellWidth, 0, i*cellWidth, SCREEN_HEIGHT);
}

void GridGame::drawRect(SDL_Rect r, rgba c)
{
    SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
    SDL_RenderFillRect(renderer, &r);
}

void GridGame::mapGrid(rgba c)
{
    if (map) //safeguard
    {
        int cellWidth = SCREEN_WIDTH / map->xSize();
        int cellHeight = SCREEN_HEIGHT / map->ySize();
        for (int i = 0; i < map->ySize(); i++){
            for (int j = 0; j < map->xSize(); j++){
                if (map->getTileAt(j, i))
                {
                    SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
                    SDL_Rect square = { j * cellWidth, i*cellHeight, cellWidth, cellHeight};
                    SDL_RenderFillRect(renderer, &square);
                }
            }
        }
    }
    
}