#include "sdlgame.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// Game class implementation

double Game::frameTime()
{
    oldTime = time;
    time = SDL_GetTicks();
    return (time - oldTime) / 1000.0;
}

// Sets screen color
void Game::clrScreen(Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
    SDL_SetRenderDrawColor(renderer, r, g, b, a);
    SDL_RenderClear(renderer);
}

bool Game::loadImage(std::vector<unsigned char>& image, const std::string& filename, int& x, int&y)
{
    int n;
    unsigned char* data = stbi_load(filename.c_str(), &x, &y, &n, 4);
    if (data != nullptr)
    {
        image = std::vector<unsigned char>(data, data + x * y * 4);
    }
    stbi_image_free(data);
    return (data != nullptr);
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

//Simple DDA
CollisionEvent GridGame::ddaRaycast(Point start, double angle)
{
    double angleRadians = angle * M_PI / 180.0;
    //using point as 2d vector to keep clean
    Point rayDir = { cos(angleRadians), sin(angleRadians) };
    Point rayUnitStepSize = { sqrt( 1 + (rayDir.y / rayDir.x) * (rayDir.y / rayDir.x)), sqrt( 1 + (rayDir.x / rayDir.y) * (rayDir.x / rayDir.y)) };
    Point mapCheck = { floor(start.x), floor(start.y) };
    
    Point rayLength;
    Point step;
    if (rayDir.x < 0) 
    {
        step.x = -1;
        rayLength.x = (start.x - mapCheck.x) * rayUnitStepSize.x;
    }
    else
    {
        step.x = 1;
        rayLength.x = (mapCheck.x + 1 - start.x) * rayUnitStepSize.x;
    } 
    if (rayDir.y < 0) 
    {
        step.y = -1;
        rayLength.y = (start.y - mapCheck.y) * rayUnitStepSize.y;
    }
    else 
    {
        step.y = 1;
        rayLength.y = (mapCheck.y + 1 - start.y) * rayUnitStepSize.y;
    }
    bool tileFound = false;
    int maxDistance = (map->xSize() > map->ySize()) ? map->xSize() : map->ySize();
    double distance = 0;
    int side;
    while (!tileFound && distance < maxDistance)
    {
        if (rayLength.x < rayLength.y)
        {
            mapCheck.x += step.x;
            distance = rayLength.x;
            rayLength.x += rayUnitStepSize.x;
            side = 0;
        }
        else
        {
            mapCheck.y += step.y;
            distance = rayLength.y;
            rayLength.y += rayUnitStepSize.y;
            side = 1;
        }
        if (mapCheck.x >= 0 && mapCheck.x < map->xSize() && mapCheck.y >= 0 && mapCheck.y < map->ySize())
        {
            if (map->getTileAt(mapCheck.x, mapCheck.y))
            {
                return {true, start + rayDir * distance, side, distance * cos(angleRadians - getAngle()*M_PI/180), map->getTileAt(mapCheck.x, mapCheck.y)}; //code fixes fish eye effect
            }
        }
        else return CollisionEvent(); //invalid
    }
    return CollisionEvent(); //invalid
}

void GridGame::setPlayerPos(Point p)
{
    if(map) //so you don't crash :)
        if (!map->getTileAt(p.x, p.y))
            playerPos = p;
        else if (!map->getTileAt(p.x, playerPos.y)) //If the player is pushing against the wall these two lines let them slide against the wall instead of just sticking
            playerPos.x = p.x;
        else if (!map->getTileAt(playerPos.x, p.y))
            playerPos.y = p.y;
}

//Wolf3d esk renderer
void GridGame::pseudo3dRender(int FOV, double wallheight)
{
    if (!currentTextureSet) //Draws red shaded walls if no current texture set
    {
        FOV /= 2;
        SDL_SetRenderDrawColor(renderer, 74, 74, 74, 255);
        SDL_RenderClear(renderer);
        for (int i = 0; i < SCREEN_WIDTH; i++)
        {
            double scanDir = 2*i/(double)SCREEN_WIDTH - 1; // -1 ---- 0 ---- 1 for the scan across the screen
            CollisionEvent collision = ddaRaycast(getPlayerPos(), getAngle() + FOV * scanDir);
            int lineHeight = (int)(wallheight*(SCREEN_HEIGHT / collision.perpWallDist));
            int drawStart = -lineHeight / 2 + SCREEN_HEIGHT / 2;
            if (drawStart < 0) drawStart = 0;
            int drawEnd = lineHeight / 2 + SCREEN_HEIGHT / 2;
            if (drawEnd > SCREEN_HEIGHT) drawEnd = SCREEN_HEIGHT; 
            Uint8 r = (collision.sideHit) ? 255 : 175;
            SDL_SetRenderDrawColor(renderer, r, 0, 0, 255);
            SDL_RenderDrawLine(renderer, i, drawStart, i, drawEnd);
            //std::cout << collision.intersect.x << " " << collision.intersect.y << std::endl; //debug
            // int textureToRender = map->getTileAt((int)collision.intersect.x, (int)collision.intersect.y);
            // double texCoord = collision.intersect.x - floor(collision.intersect.x);
            // std::cout << collision.tileData << std::endl;
        }
    }
    else //If texture set draws textures
    {
        FOV /= 2;
        SDL_SetRenderDrawColor(renderer, 74, 74, 74, 255);
        SDL_RenderClear(renderer);
        for (int i = 0; i < SCREEN_WIDTH; i++)
        {
            double scanDir = 2*i/(double)SCREEN_WIDTH - 1; // -1 ---- 0 ---- 1 for the scan across the screen
            CollisionEvent collision = ddaRaycast(getPlayerPos(), getAngle() + FOV * scanDir);
            int lineHeight = (int)(wallheight*(SCREEN_HEIGHT / collision.perpWallDist));
            int drawStart = -lineHeight / 2 + SCREEN_HEIGHT / 2;
            if (drawStart < 0) drawStart = 0;
            int drawEnd = lineHeight / 2 + SCREEN_HEIGHT / 2;
            if (drawEnd > SCREEN_HEIGHT) drawEnd = SCREEN_HEIGHT; 
            double texCoord = collision.intersect.x - floor(collision.intersect.x);
            int textureToRender = collision.tileData;
            if (textureToRender)
            {
                int texX = static_cast<int>(texCoord * currentTextureSet->widthHeightAt(textureToRender-1).first);
                for (int y = drawStart; y < drawEnd; y++)
                {
                    int texY = (((y * 2 - SCREEN_HEIGHT + lineHeight) << 6) / lineHeight) / 2;
                    rgba textureColor = currentTextureSet->colorAt(textureToRender-1, texX, texY);
                    SDL_SetRenderDrawColor(renderer, textureColor.r, textureColor.g, textureColor.b, textureColor.a);
                    SDL_RenderDrawPoint(renderer, i, y);
                }
            }
        }
    }
}

//Texture handler constructor takes in vector of filenames and loads them in
TextureHandler::TextureHandler(std::vector<std::string> in)
{
    for (std::string filename : in)
    {
        int width, height;
        std::vector<unsigned char> image;
        bool success = Game::loadImage(image, filename, width, height);
        if (!success)
        {
            std::cout << "Error loading image " + filename + "\n";
        }
        loadedTextures.push_back(image);
        loadedTextureSizes.push_back(std::make_pair(width, height));
    }
}

rgba TextureHandler::colorAt(int textureIndex, int x, int y)
{
    const int RGBA = 4; //This might change if you change the loadimage function
    int r, g, b, a;
    int index = RGBA * ( y * widthHeightAt(textureIndex).first + x);
    r = static_cast<int>(loadedTextures.at(textureIndex)[index + 0]);
    g = static_cast<int>(loadedTextures.at(textureIndex)[index + 1]);
    b = static_cast<int>(loadedTextures.at(textureIndex)[index + 2]);
    a = static_cast<int>(loadedTextures.at(textureIndex)[index + 3]);
    // std::cout << r << " "
    //           << g << " "
    //           << b << " "
    //           << a << '\n';
    return { r, g, b, a };
}