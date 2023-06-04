#include "sdlgame.hpp"

// Game class implementation

// Sets screen color immediately
void Game::clrScreen(Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
    SDL_SetRenderDrawColor(renderer, r, g, b, a);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);
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