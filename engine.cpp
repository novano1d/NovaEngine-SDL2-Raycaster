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
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <SDL2/SDL_ttf.h>
// Game class implementation

double Game::frameTime()
{
    oldTime = time;
    time = SDL_GetPerformanceCounter();
    double ticktime = static_cast<float>(time - oldTime) / static_cast<float>(SDL_GetPerformanceFrequency());
    ticks += TICKS * ticktime;
    if (ticks >= UINT64_MAX) ticks = 0; //reset ticks if too big
    return ticktime;
} 

// Sets screen color
void Game::clrScreen(Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
    SDL_SetRenderDrawColor(renderer, r, g, b, a);
    SDL_RenderClear(renderer);
}

//images must be in an './images/' subfolder
bool nva::loadImage(std::vector<unsigned char>& image, const std::string& filename, int& x, int&y)
{ 
    int n;
    unsigned char* data = stbi_load(("./images/" + filename).c_str(), &x, &y, &n, 4);
    if (data != nullptr)
    {
        image = (std::vector<unsigned char>(data, data + x * y * 4));
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
inline CollisionEvent GridGame::ddaRaycast(Point start, double angle)
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
            else if (map->getDoorTileAt(mapCheck.x, mapCheck.y).exists)
            {
                //if it's a door we need to register a hit at a different point to render a thin wall and provide animation
                double doorProgress = map->getDoorTileAt(mapCheck.x, mapCheck.y).doorProgress;
                Point intersection = start + rayDir * distance;
                if (map->getDoorTileAt(mapCheck.x, mapCheck.y).orientation) //horiz
                {
                    if (intersection.x >= mapCheck.x + 0.0001 && intersection.x <= mapCheck.x - 0.0001 + doorProgress) //rounding error sigh
                    {
                        return {2, start + rayDir * distance, side, distance * cos(angleRadians - getAngle()*M_PI/180), map->getDoorTileAt(mapCheck.x, mapCheck.y).texIndex, map->getDoorTileAt(mapCheck.x, mapCheck.y).doorProgress}; 
                    }
                }
                else //vert
                {
                    if (intersection.y >= mapCheck.y + 0.0001 && intersection.y <= mapCheck.y - 0.0001 + doorProgress)
                    {
                        return {2, start + rayDir * distance, side, distance * cos(angleRadians - getAngle()*M_PI/180), map->getDoorTileAt(mapCheck.x, mapCheck.y).texIndex, map->getDoorTileAt(mapCheck.x, mapCheck.y).doorProgress}; 
                    }
                }
            }
        }
        else return CollisionEvent(); //invalid
    }
    return CollisionEvent(); //invalid
}

void GridGame::setPlayerPos(Point p)
{
    const float WALL_CLOSENESS = 0.1; // how close the player can get (in game grid units) to the wall  
    if(map)
    {
        bool stuckOnVerticalWall = map->getTileAt(playerPos.x - WALL_CLOSENESS, playerPos.y) || map->getTileAt(playerPos.x + WALL_CLOSENESS, playerPos.y);
        bool stuckOnHorizontalWall = map->getTileAt(playerPos.x, playerPos.y - WALL_CLOSENESS) || map->getTileAt(playerPos.x, playerPos.y + WALL_CLOSENESS);
        
        bool stuckOnVerticalDoor = (map->getDoorTileAt(playerPos.x - WALL_CLOSENESS, playerPos.y).doorState == true) || (map->getDoorTileAt(playerPos.x + WALL_CLOSENESS, playerPos.y).doorState == true);
        bool stuckOnHorizontalDoor = (map->getDoorTileAt(playerPos.x, playerPos.y - WALL_CLOSENESS).doorState == true) || (map->getDoorTileAt(playerPos.x, playerPos.y + WALL_CLOSENESS).doorState == true);
        if ((!stuckOnVerticalWall && !stuckOnVerticalDoor) || (stuckOnVerticalWall && ((p.x < playerPos.x && !map->getTileAt(p.x - WALL_CLOSENESS, p.y)) || (p.x > playerPos.x && !map->getTileAt(p.x + WALL_CLOSENESS, p.y)))) || (stuckOnVerticalDoor && ((p.x < playerPos.x && !(map->getDoorTileAt(p.x - WALL_CLOSENESS, p.y).doorState == true)) || (p.x > playerPos.x && !(map->getDoorTileAt(p.x + WALL_CLOSENESS, p.y).doorState == true)))))
            playerPos.x = p.x;
        if ((!stuckOnHorizontalWall && !stuckOnHorizontalDoor) || (stuckOnHorizontalWall && ((p.y < playerPos.y && !map->getTileAt(p.x, p.y - WALL_CLOSENESS)) || (p.y > playerPos.y && !map->getTileAt(p.x, p.y + WALL_CLOSENESS)))) || (stuckOnHorizontalDoor && ((p.y < playerPos.y && !(map->getDoorTileAt(p.x, p.y - WALL_CLOSENESS).doorState == true)) || (p.y > playerPos.y && !(map->getDoorTileAt(p.x, p.y + WALL_CLOSENESS).doorState == true)))))
            playerPos.y = p.y;
        std::cout << "at: " << playerPos.x << " " << playerPos.y << " " << getAngle() << std::endl;
        
    }
}


// DEPRECIATED or bug testing only
void GridGame::pseudo3dRender(int FOV, double wallheight)
{
    FOV /= 2;
    SDL_SetRenderDrawColor(renderer, 74, 74, 74, 255);
    SDL_RenderClear(renderer);
    for (int i = 0; i < SCREEN_WIDTH; i++)
    {
        double scanDir = 2 * i / (double)SCREEN_WIDTH - 1; // -1 ---- 0 ---- 1 for the scan across the screen
        CollisionEvent collision = ddaRaycast(getPlayerPos(), getAngle() + FOV * scanDir);
        int lineHeight = (int)(wallheight * (SCREEN_HEIGHT / collision.perpWallDist));
        int drawStart = -lineHeight / 2 + SCREEN_HEIGHT / 2;
        if (drawStart < 0)
            drawStart = 0;
        int drawEnd = lineHeight / 2 + SCREEN_HEIGHT / 2;
        if (drawEnd > SCREEN_HEIGHT)
            drawEnd = SCREEN_HEIGHT;
        Uint8 r = (collision.sideHit) ? 255 : 175;
        SDL_SetRenderDrawColor(renderer, r, 0, 0, 255);
        SDL_RenderDrawLine(renderer, i, drawStart, i, drawEnd);
        // std::cout << collision.intersect.x << " " << collision.intersect.y << std::endl; //debug
        // int textureToRender = map->getTileAt((int)collision.intersect.x, (int)collision.intersect.y);
        // double texCoord = collision.intersect.x - floor(collision.intersect.x);
        // std::cout << collision.tileData << std::endl;
    }
    SDL_RenderPresent(renderer); // fast enough we don't need a buffer
}



//returns the ID of the entity hit
int GridGame::shoot(Point p, double a)
{
    CollisionEvent e = ddaRaycast(p, a);
    std::vector<std::pair<int, int>> distIndexVec;
    a = fmod(a, 360);
    a *= M_PI/180;
    double xcom = cos(a) * 0.01; //change so we increment
    double ycom = sin(a) * 0.01;
    //auto sprites = map->getSprites();
    auto entities = map->getEntities()->getEntityVec();
    double RANGE = 1000;
    int index = 0;
    Point check = p;
    [&] { //lambda to avoid goto statement
        for (int t = 0; t < entities.size(); t++)
        {
            Entity *e = entities[t]; //modify so looping through entities, not all sprites
            check = p;
            double d = 0;
            for (int i = 0; i < RANGE; i++)
            {
                if (nva::checkCirc((*e).pos.x, (*e).pos.y, (*e).radius, check.x, check.y))
                {
                    distIndexVec.push_back(std::make_pair(d, (*e).ID));
                    return;
                }
                check.x += xcom;
                check.y += ycom;
                d += hypot(xcom, ycom);
            }
        }
    }();
    std::sort(distIndexVec.begin(), distIndexVec.end());
    if (distIndexVec.size() == 0) return -1;
    if (distIndexVec.at(0).first < e.perpWallDist) //if the bullet is occluded we don't want to reg a hit
        return distIndexVec.at(0).second;
    return -1;
}

//speed could almost certainly be improved with multithreading or decreasing # of raycasts
void GridGame::pseudo3dRenderTextured(int FOV, double wallheight)
{
    // Calculate the render dimensions
    const int renderWidth = INTERNAL_RENDER_RES_HORIZ;
    const int renderHeight = INTERNAL_RENDER_RES_VERT;
    double ZBuffer[renderWidth]; // store Z distances for sprite rendering (necessary for occlusion)
    if (textureBuffer == nullptr)
        textureBuffer = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, renderWidth, renderHeight);
    std::vector<std::thread> threads;
    const int sectionWidth = renderWidth / nva::MAX_THREADS;
    int startX = 0;
    Uint32* pixels;
    int pitch;
    SDL_LockTexture(textureBuffer, nullptr, reinterpret_cast<void**>(&pixels), &pitch);
    Uint8 rshift = format->Rshift;
    Uint8 gshift = format->Gshift;
    Uint8 bshift = format->Bshift;
    Uint8 ashift = format->Ashift;
    FOV /= 2;
    //wall casting
    for (int i = 0; i < nva::MAX_THREADS; i++)
    {
        int endX = (i == nva::MAX_THREADS - 1) ? renderWidth : startX + sectionWidth;
        threads.push_back(std::thread([&]{
            //std::mutex mtx;
            //mtx.lock();
            for (int i = startX; i < endX; i++)
    {
        //change scandir in order to fix the spherical distortion
        //double scanDir = 2 * i / static_cast<double>(renderWidth) - 1; // -1 ---- 0 ---- 1 for the scan across the screen
        double opp = i - renderWidth / 2.0;
        double adj = renderWidth / (tan((FOV * M_PI / 180)));
        double scanDir = atan(opp / adj); // Updated scanDir
        CollisionEvent collision = ddaRaycast(getPlayerPos(), angle + FOV * scanDir);
        //could probably change perpwalldist in order to get infinitely thin walls
        ZBuffer[i] = collision.perpWallDist; //set zbuffer value
        int lineHeight = static_cast<int>(wallheight * (renderHeight / collision.perpWallDist));
        int drawStart = -lineHeight / 2 + renderHeight / 2;
        if (drawStart < 0) drawStart = 0;
        int drawEnd = lineHeight / 2 + renderHeight / 2;
        if (drawEnd > renderHeight) drawEnd = renderHeight;
        double texCoord;
        const Uint32 black = SDL_MapRGBA(format, 0, 0, 0, 255);
        if (collision.sideHit)
            texCoord = collision.intersect.x - static_cast<int>(collision.intersect.x);
        else
            texCoord = collision.intersect.y - static_cast<int>(collision.intersect.y);
        int textureToRender = collision.tileData;
        int texX = static_cast<int>(texCoord * currentTextureSet->widthHeightAt(textureToRender - 1).first);
        texX = nva::clamp<int>(texX, 0, currentTextureSet->widthHeightAt(textureToRender - 1).first);
        if (collision.hit == 2) // door we need to offset the texture according to the progress
        {
            texX = static_cast<int>((texCoord + (1 - collision.doorProgress)) * currentTextureSet->widthHeightAt(textureToRender - 1).first);
            texX = nva::clamp<int>(texX, 0, currentTextureSet->widthHeightAt(textureToRender - 1).first);
        }
        
        double lightVal = nva::BRIGHTNESS - map->getLightTileAt(collision.intersect.x + 0.001, collision.intersect.y + 0.001) * nva::BRIGHTNESS;
        if (lightVal == 0) lightVal = 1;
        if (textureToRender)
        {
            /*
            
                Walls

            */
            for (int y = drawStart; y < drawEnd; y++)
            {
                int texY = (((y * 2 - renderHeight + lineHeight) * currentTextureSet->widthHeightAt(textureToRender - 1).second) / lineHeight) / 2;
                texY = nva::clamp<int>(texY, 0, currentTextureSet->widthHeightAt(textureToRender - 1).second);
                rgba textureColor;
                textureColor = currentTextureSet->colorAt(textureToRender - 1, texX, texY);
                pixels[y * renderWidth + i] = ((int)(textureColor.r / lightVal) << rshift) |
                                                ((int)(textureColor.g / lightVal) << gshift) |
                                                ((int)(textureColor.b / lightVal) << bshift) |
                                                (textureColor.a << ashift);
            }
            //floor casting
            for (int y = drawEnd + 1; y <= renderHeight; y++)
            {
                // Calculate the current distance from the player to the floor/ceiling
                double currentDist = renderHeight / (2.0 * y - renderHeight);
                double weight = currentDist / collision.perpWallDist;
                double floorX = weight * collision.intersect.x + (1 - weight) * playerPos.x;
                double floorY = weight * collision.intersect.y + (1 - weight) * playerPos.y;
                int ceilTex = map->getCeilingTileAt(floorX, floorY);
                int floorTex = map->getFloorTileAt(floorX, floorY);
                double lightVal = nva::BRIGHTNESS - map->getLightTileAt(floorX, floorY) * nva::BRIGHTNESS;
                if (lightVal == 0) lightVal = 1;
                rgba ctex;
                if (ceilTex == SKY)
                {
                    lightVal = 1;
                    int cw = currentTextureSet->widthHeightAt(map->getSkyTexture()).first;
                    int ch = currentTextureSet->widthHeightAt(map->getSkyTexture()).second;
                    if (angle < 0) angle += 360;
                    int skyOffset = static_cast<int>(angle * SKYSCALEFACTOR) % cw;
                    int ceilTexX = (i + skyOffset) * (cw / renderWidth) % cw;
                    int ceilTexY = y * (ch / renderHeight) % ch;
                    ceilTexX = nva::clamp<int>(ceilTexX, 0, cw);
                    ceilTexY = nva::clamp<int>(ceilTexY, 0, ch);
                    ctex = currentTextureSet->colorAt(map->getSkyTexture(), ceilTexX, ceilTexY);
                }
                else
                {
                    int cw = currentTextureSet->widthHeightAt(ceilTex).first;
                    int ch = currentTextureSet->widthHeightAt(ceilTex).second;
                    int ceilTexX = static_cast<int>(floorX * cw) % cw;
                    int ceilTexY = static_cast<int>(floorY * ch) % ch;
                    ceilTexX = nva::clamp<int>(ceilTexX, 0, cw);
                    ceilTexY = nva::clamp<int>(ceilTexY, 0, ch);
                    ctex = currentTextureSet->colorAt(ceilTex, ceilTexX, ceilTexY);
                }
                int fw = currentTextureSet->widthHeightAt(floorTex).first;
                int floorTexX = static_cast<int>(floorX * fw) % fw;
                int fh = currentTextureSet->widthHeightAt(floorTex).second;
                int floorTexY = static_cast<int>(floorY * fh) % fh;
                floorTexX = nva::clamp<int>(floorTexX, 0, fw);
                floorTexY = nva::clamp<int>(floorTexY, 0, fh);
                rgba ftex = currentTextureSet->colorAt(floorTex, floorTexX, floorTexY);
                pixels[(y-1) * renderWidth + i] = ((int)(ftex.r /lightVal) << rshift) | //floor
                                                        ((int)(ftex.g / lightVal) << gshift) |
                                                        ((int)(ftex.b / lightVal) << bshift) |
                                                        (ftex.a << ashift);
                pixels[(renderHeight - y) * renderWidth + i] = ((int)(ctex.r / lightVal) << rshift) | //ceiling
                                                               ((int)(ctex.g / lightVal) << gshift) |
                                                               ((int)(ctex.b / lightVal) << bshift) |
                                                               (ctex.a << ashift);
            }
        }
        else
        {
            for (int y = 0; y < renderHeight; y++)
            {
                pixels[y * renderWidth + i] = black;
            }
        }
        
    }
        //mtx.unlock();
        startX += sectionWidth;
        }));
    }
    
    for (auto& thread : threads)
    {
        thread.join();
    }
    /*
    
        SPRITES RENDERING

    */
    std::vector<Sprite*> temp = (map->getSprites()); //pointer so we don't sort each time :)
    //sort sprites by distance from player
    //std::vector<double> distance; //parallel distance vector
    // std::transform(temp.begin(), temp.end(), distance.begin(), [this](Sprite s){ return hypot(s.x - getPlayerPos().x, s.y - getPlayerPos().y); });
    // std::sort(distance.begin(), distance.end(), [](double &a, double &b){ return a > b; }); //sort distances to sprites in descending order
    std::sort(temp.begin(), temp.end(), [this](Sprite *a, Sprite *b){
    return hypot(a->x - getPlayerPos().x, a->y - getPlayerPos().y) > hypot(b->x - getPlayerPos().x, b->y - getPlayerPos().y);
    });

    //rendering
    
/*
double opp = i - renderWidth / 2.0;
double adj = renderWidth / (tan((FOV * M_PI / 180)));
double scanDir = atan(opp / adj); // Updated scanDir

SPRITES ARE FIXED!!!
I HAVE NO IDEA HOW I FIXED IT, BUT IT WORKS NOW
*/

    double planeLength = tan((FOV/2.0) * M_PI / 180) * 2.0; 
    double planeX = -sin(angle * M_PI / 180) * planeLength; 
    double planeY = cos(angle * M_PI / 180) * planeLength;
    for (auto it = temp.begin(); it != temp.end(); it++)
    {
        double spriteX = (*it)->x - getPlayerPos().x;
        double spriteY = (*it)->y - getPlayerPos().y;
        double invDet = 1.0 / (planeX * sin(angle * M_PI / 180) - cos(angle * M_PI / 180) * planeY);
        double transformX = invDet * (sin(angle * M_PI / 180) * spriteX - cos(angle * M_PI / 180) * spriteY);
        double transformY = invDet * (-planeY * spriteX + planeX * spriteY);
        //int spriteScreenX = int((static_cast<double>(renderWidth) / 2.0) * (1.0 + transformX / transformY));
        double spriteDir = atan2(spriteY, spriteX);
        int spriteScreenX = std::round(renderWidth / 2.0 + renderWidth * tan((spriteDir * 180 / M_PI - angle) * M_PI / 180) / (tan(FOV * M_PI / 180)));
        int spriteHeight = abs(int(renderHeight / transformY));
        int drawStartY = -spriteHeight / 2 + renderHeight / 2;
        if(drawStartY < 0) drawStartY = 0;
        int drawEndY = spriteHeight / 2 + renderHeight / 2;
        if(drawEndY >= renderHeight) drawEndY = renderHeight - 1;
        int spriteWidth = abs(int(renderHeight / transformY));
        int drawStartX = -spriteWidth / 2 + spriteScreenX;
        if(drawStartX < 0) drawStartX = 0;
        int drawEndX = spriteWidth / 2 + spriteScreenX;
        if(drawEndX >= renderWidth) drawEndX = renderWidth - 1;
        int texSelect = 0; //default
        double lightVal = nva::BRIGHTNESS - map->getLightTileAt((*it)->x, (*it)->y) * nva::BRIGHTNESS;
        if (lightVal == 0) lightVal = 1;
        if ((*it)->multiAngle && (*it)->animated)
        {
            const int numOrientations = 8; // Eight orientations

            // Calculate the angle between player and sprite
            //int diff = (angle) - it->angle /* calculate the angle between player and sprite */;
            double diffr = std::atan2(spriteY, spriteX);
            // Convert the angle to degrees
            double diff = diffr * 180 / M_PI;
            int orientationIndex = 0; // Default orientation index

            // Calculate the angle per orientation (in degrees)
            double orientationAngle = 360.0 / numOrientations;

            // Normalize the angle difference to be within [0, 360) degrees
            diff += (*it)->angle; //add angle of sprite
            diff = fmod(diff + 360, 360);

            // Determine the orientation index based on the angle
            if (diff >= 0 * orientationAngle && diff < 1 * orientationAngle) {
                orientationIndex = 0; // Facing front
            } else if (diff >= 1 * orientationAngle && diff < 2 * orientationAngle) {
                orientationIndex = 1; // Facing front-left
            } else if (diff >= 2 * orientationAngle && diff < 3 * orientationAngle) {
                orientationIndex = 2; // Facing left
            } else if (diff >= 3 * orientationAngle && diff < 4 * orientationAngle) {
                orientationIndex = 3; // Facing right (wrap around)
            } else if (diff >= 4 * orientationAngle && diff < 5 * orientationAngle) {
                orientationIndex = 4; // Facing front-right
            } else if (diff >= 5 * orientationAngle && diff < 6 * orientationAngle) {
                orientationIndex = 5; // Facing front
            } else if (diff >= 6 * orientationAngle && diff < 7 * orientationAngle) {
                orientationIndex = 6; // Facing front-left
            } else if (diff >= 7 * orientationAngle && diff < 8 * orientationAngle) {
                orientationIndex = 7; // Facing left
            }

            //Use the selected orientation index to get the texture for rendering
            int reelSelect = orientationIndex;
            //reel determined from different angle
            auto reel = (*it)->animIndexesAngled[reelSelect];
            if (ticks - (*it)->lastSpriteTick >= reel[2*(*it)->curAnimIndex])
            {
                (*it)->curAnimIndex += 1;
                (*it)->lastSpriteTick = ticks; 
                if ((*it)->curAnimIndex >= reel.size()/2) (*it)->curAnimIndex = 0;
            }
            texSelect = reel[2*(*it)->curAnimIndex + 1];
        }
        else if ((*it)->multiAngle)
        {
            const int numOrientations = 8; // Eight orientations

            // Calculate the angle between player and sprite
            //int diff = (angle) - it->angle /* calculate the angle between player and sprite */;
            double diffr = std::atan2(spriteY, spriteX);
            // Convert the angle to degrees
            double diff = diffr * 180 / M_PI;
            int orientationIndex = 0; // Default orientation index

            // Calculate the angle per orientation (in degrees)
            double orientationAngle = 360.0 / numOrientations;

            // Normalize the angle difference to be within [0, 360) degrees
            diff += (*it)->angle; //add angle of sprite
            diff = fmod(diff + 360, 360);

            // Determine the orientation index based on the angle
            if (diff >= 0 * orientationAngle && diff < 1 * orientationAngle) {
                orientationIndex = (*it)->angleIndexes[0]; // Facing front
            } else if (diff >= 1 * orientationAngle && diff < 2 * orientationAngle) {
                orientationIndex = (*it)->angleIndexes[1]; // Facing front-left
            } else if (diff >= 2 * orientationAngle && diff < 3 * orientationAngle) {
                orientationIndex = (*it)->angleIndexes[2]; // Facing left
            } else if (diff >= 3 * orientationAngle && diff < 4 * orientationAngle) {
                orientationIndex = (*it)->angleIndexes[3]; // Facing right (wrap around)
            } else if (diff >= 4 * orientationAngle && diff < 5 * orientationAngle) {
                orientationIndex = (*it)->angleIndexes[4]; // Facing front-right
            } else if (diff >= 5 * orientationAngle && diff < 6 * orientationAngle) {
                orientationIndex = (*it)->angleIndexes[5]; // Facing front
            } else if (diff >= 6 * orientationAngle && diff < 7 * orientationAngle) {
                orientationIndex = (*it)->angleIndexes[6]; // Facing front-left
            } else if (diff >= 7 * orientationAngle && diff < 8 * orientationAngle) {
                orientationIndex = (*it)->angleIndexes[7]; // Facing left
            }

            //Use the selected orientation index to get the texture for rendering
            texSelect = orientationIndex;
        }
        else if ((*it)->animated)
        {
            auto reel = (*it)->animIndexes;
            if (ticks - (*it)->lastSpriteTick >= reel[2*(*it)->curAnimIndex])
            {
                (*it)->curAnimIndex += 1;
                (*it)->lastSpriteTick = ticks;
                if ((*it)->curAnimIndex >= reel.size()/2) (*it)->curAnimIndex = 0;
            }
            texSelect = reel[2*(*it)->curAnimIndex + 1];
        }

        else texSelect = (*it)->texIndex;
        
        for(int stripe = drawStartX; stripe < drawEndX; stripe++)
        {
            int texX = int(256 * (stripe - (-spriteWidth / 2 + spriteScreenX)) * currentTextureSet->widthHeightAt(texSelect).first / spriteWidth) / 256;
            texX = nva::clamp<int>(texX, 0, currentTextureSet->widthHeightAt(texSelect).first);
            if(transformY > 0 && stripe > 0 && stripe < renderWidth && transformY < ZBuffer[stripe])
            {
                for(int y = drawStartY; y < drawEndY; y++)
                {
                    int d = (y - renderHeight / 2) * 256 + spriteHeight * 128;
                    int texY = ((d * currentTextureSet->widthHeightAt(texSelect).second) / spriteHeight) / 256;
                    texY = nva::clamp<int>(texY, 0, currentTextureSet->widthHeightAt(texSelect).second);
                    rgba textureColor;
                    textureColor = currentTextureSet->colorAt(texSelect, texX, texY);
                    if(textureColor.a != 0) // If the pixel is not transparent
                        pixels[y * renderWidth + stripe] =  ((int)(textureColor.r / lightVal) << rshift) |
                                                            ((int)(textureColor.g / lightVal) << gshift) |
                                                            ((int)(textureColor.b / lightVal) << bshift) |
                                                            (textureColor.a << ashift);
                }
            }
        }
    }

    SDL_UnlockTexture(textureBuffer);

    // Calculate the target area to maintain aspect ratio
    int windowWidth, windowHeight;
    SDL_GetWindowSize(window, &windowWidth, &windowHeight);
    float windowAspectRatio = static_cast<float>(windowWidth) / windowHeight;
    float bufferAspectRatio = static_cast<float>(renderWidth) / renderHeight;

    SDL_Rect targetRect;
    if (windowAspectRatio > bufferAspectRatio)
    {
        // Window is wider, keep the height and center horizontally
        int targetWidth = windowHeight * bufferAspectRatio;
        targetRect.x = (windowWidth - targetWidth) / 2;
        targetRect.y = 0;
        targetRect.w = targetWidth;
        targetRect.h = windowHeight;
    }
    else
    {
        // Window is taller or same aspect ratio, keep the width and center vertically
        int targetHeight = windowWidth / bufferAspectRatio;
        targetRect.x = 0;
        targetRect.y = (windowHeight - targetHeight) / 2;
        targetRect.w = windowWidth;
        targetRect.h = targetHeight;
    }
    // Clear the window with black color
    //SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // Render the back buffer texture in the window
    SDL_RenderCopy(renderer, textureBuffer, nullptr, &targetRect);
    /*
    
            UI HERE DOWN THEN RENDER PRESENT
    
    */

    //Render gun
    const int GUNSCALE = 4;
    std::vector<unsigned char> imageData = currentTextureSet->getLoadedTextures()[gunIndex];
    std::pair<int, int> dimensions = currentTextureSet->widthHeightAt(gunIndex);
    int32_t width = dimensions.first;
    int32_t height = dimensions.second;
    int screenWidth, screenHeight;
    SDL_GetRendererOutputSize(renderer, &screenWidth, &screenHeight); 
    int x = (screenWidth - (width*GUNSCALE)) / 2; // Horizontal position for center alignment.
    int y = screenHeight - height*GUNSCALE; // Vertical position for bottom alignment.
    SDL_Rect dstrect = { x, y, width*GUNSCALE, height*GUNSCALE };
    int32_t bytesPerPixel = imageData.size() / (width * height);
    int32_t pit = width * bytesPerPixel;
    pit = (pit + 3) & ~3; // Align the pitch to 4 bytes
    int32_t Rmask = 0x000000FF;
    int32_t Gmask = 0x0000FF00;
    int32_t Bmask = 0x00FF0000;
    int32_t Amask = (bytesPerPixel == 4) ? 0xFF000000 : 0;
    auto cachedGunSurface = SDL_CreateRGBSurfaceFrom((imageData.data()), width, height, bytesPerPixel * 8, pit, Rmask, Gmask, Bmask, Amask);
    auto cachedGunTex = SDL_CreateTextureFromSurface(renderer, cachedGunSurface);
    SDL_RenderCopy(renderer, cachedGunTex, NULL, &dstrect);
    SDL_FreeSurface(cachedGunSurface);
    SDL_DestroyTexture(cachedGunTex);

    // SDL_Point point = {0, 0};
    // FOX_RenderText(font, (const Uint8*)"Health: 100", &point);

    // Present the rendered frame
    SDL_RenderPresent(renderer);
}


GridGame::~GridGame()
{
    SDL_DestroyTexture(textureBuffer);
}

//Texture handler constructor takes in vector of filenames and loads them in
//Also takes in the renderer to handle a loading screen
TextureHandler::TextureHandler(SDL_Renderer* renderer, std::vector<std::string> in)
{
    int width, height;
    SDL_Rect progressBar;
    progressBar.x = nva::SCREEN_WIDTH / 4;
    progressBar.y = nva::SCREEN_HEIGHT / 2;
    progressBar.h = 20;

    // In your loading loop, update the progress bar width based on loading progress
    int progress = 0;

    // Set render color to white (for the progress bar)

    // Render the progress bar
    for (auto i = in.begin(); i != in.end(); i++)
    {
        progress += 1;
        progressBar.w = (nva::SCREEN_WIDTH / 2) * ((float)progress / in.size());
        std::string filename = *i;
        std::vector<unsigned char> image;
        bool success = nva::loadImage(image, filename, width, height);
        if (!success)
        {
            std::cout << "Error loading image " + filename + "\n";
        }
        loadedTextures.emplace_back(image);
        loadedTextureSizes.emplace_back(std::make_pair(width, height));
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
        SDL_RenderClear(renderer);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderFillRect(renderer, &progressBar);
        std::string percentage = std::to_string((progress / (float)in.size()) * 100) + "%";
        SDL_Color color = { 255, 255, 255 };  // white color_
        auto font = TTF_OpenFont("./fonts/SuboleyaRegular.ttf", 25);
        if (font == nullptr) {
            std::cerr << "Error loading font.";
        }
        SDL_Surface* surface = TTF_RenderText_Solid(font, percentage.c_str(), color);
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        int textW = 0;
        int textH = 0;
        SDL_QueryTexture(texture, NULL, NULL, &textW, &textH);
        SDL_Rect dstrect = { (nva::SCREEN_WIDTH - textW) / 2, progressBar.y - textH, textW, textH };
        SDL_RenderCopy(renderer, texture, NULL, &dstrect);
        SDL_DestroyTexture(texture);
        SDL_FreeSurface(surface);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
        SDL_RenderPresent(renderer);
        TTF_CloseFont(font);
    }
    
}

//might prove to be a bottleneck in performance since this function is called for every pixel being rendered on the wall.... therefore we may need to reduce
//the call time as much as possible and change the loaded textures class to store in an array instead of a vector
inline rgba TextureHandler::colorAt(int textureIndex, int x, int y)
{
    const int RGBA = 4; //This might change if you change the loadimage function
    int r, g, b, a;
    int index = RGBA * ( y * widthHeightAt(textureIndex).first + x);
    r = static_cast<int>(loadedTextures[textureIndex][index + 0]);
    g = static_cast<int>(loadedTextures[textureIndex][index + 1]);
    b = static_cast<int>(loadedTextures[textureIndex][index + 2]);
    a = static_cast<int>(loadedTextures[textureIndex][index + 3]);
    // std::cout << r << " "
    //           << g << " "
    //           << b << " "
    //           << a << " "
    //           << *(Uint32*)&(loadedTextures[textureIndex][index]) << " "
    //           << SDL_MapRGBA(SDL_AllocFormat(SDL_PIXELFORMAT_RGBA8888), r, g , b , a) << " "
    //           << '\n';
    return { r, g, b, a };
}

void EntityController::createEntityAndSpriteAt(Entity *e, Sprite *s, Point pos, double radius, std::string type)
{
    e->pos = pos;
    s->x = pos.x;
    s->y = pos.y;
    int ID = eh->addEntity(e);
    IDtoIndex[ID] = eh->getEntityVec().size() - 1;
    m->addSprite(s);
}

/// @brief Removes entity and sprite by ID.
/// @param id to be deleted.
void EntityController::removeEntityAndSpriteByID(int id)
{
    auto it = IDtoIndex.find(id);
    if (it != IDtoIndex.end())
    {
        int index = it->second;
        eh->deleteEntityByID(index);
        m->removeSpriteAt(index);
        
        for (auto entry = IDtoIndex.begin(); entry != IDtoIndex.end();) {
            if (entry->second > index) {
                entry->second = entry->second - 1;
            }
            entry++;
        }
        IDtoIndex.erase(it);
    }
}

Point EntityController::getPosByID(int id)
{
    auto it = IDtoIndex.find(id);
    if (it != IDtoIndex.end())
    {
        int index = it->second;
        Sprite temp = m->getSpriteAt(index);
        Point pos = { temp.x , temp.y };
        return pos;
    }
    return {-1,-1};
}

/// @brief Updates the relative position of an entity given the entity ID.
/// @param ID Entity ID
/// @param x relative change in X
/// @param y relative change in Y
void EntityController::updateEntityRelPos(int ID, double x, double y)
{
    auto it = IDtoIndex.find(ID);
    if (it != IDtoIndex.end())
    {
        int index = it->second;
        Entity* e = eh->entityAt(index);
        Point newPos = {e->pos.x + x, e->pos.y + y};
        e->pos = newPos;
        m->getSpriteAt(index).x = newPos.x;
        m->getSpriteAt(index).y = newPos.y;
    }
}

/// @brief Deletes entity by the entity ID.
/// @param i the ID
void EntityHandler::deleteEntityByID(int i)
{
    for (Entity* e : entities)
    {
        if (e->ID == i) entities.erase(entities.begin() + i);
    }
}

Entity* EntityHandler::getEntityByID(int i)
{
    for (Entity* e : entities)
    {
        if (e->ID == i) return entityAt(i);
    }
}

Door Map::getDoorByID(int ID)
{
    for (int y = 0; y < doorMap.size(); y++)
    {
        for (int x = 0; x < doorMap[0].size(); x++)
        {
            if(doorMap[y][x].ID == ID)
            { 
                return doorMap[y][x];
            }
        }
    }
    return {0};
}

void Map::setDoorByID(int ID, Door d)
{
    for (int y = 0; y < doorMap.size(); y++)
    {
        for (int x = 0; x < doorMap[0].size(); x++)
        {
            if(doorMap[y][x].ID == ID)
            { 
                doorMap[y][x] = d;
            }
        }
    }
}

void Map::toggleDoorByID(int ID)
{
    doorsInProgress.insert(ID); // keep track so updating doors is fast
    Door tempd = getDoorByID(ID);
    if (tempd.state == DOOR_CLOSED)
    {
        tempd.state = DOOR_OPENING;
        tempd.doorState = true;
        setDoorByID(ID, tempd);
    }
    if (tempd.state == DOOR_OPEN)
    {
        tempd.state = DOOR_CLOSING;
        tempd.doorState = true;
        setDoorByID(ID, tempd);
    }
}

void Map::updateDoors(double t)
{
    if (!doorsInProgress.empty())
    {
        for (int id : doorsInProgress)
        {
            Door tempd = getDoorByID(id);
            if (tempd.state == DOOR_CLOSING)
            {
                tempd.doorProgress += t * tempd.doorTime;
                if (tempd.doorProgress >= 1)
                {
                    tempd.doorProgress = 1;
                    tempd.state = DOOR_CLOSED;
                    tempd.doorState = true;
                    doorsInProgress.erase(doorsInProgress.find(id));
                }
                setDoorByID(id, tempd);
            }
            else if (tempd.state == DOOR_OPENING)
            {
                tempd.doorProgress -= t * tempd.doorTime;
                if (tempd.doorProgress <= 0)
                {
                    tempd.doorProgress = 0;
                    tempd.state = DOOR_OPEN;
                    tempd.doorState = false;
                    doorsInProgress.erase(doorsInProgress.find(id));
                }
                setDoorByID(id, tempd);
            }
        }
    }
}

bool Map::isDoorNeighbor(Point p)
{
    Point c1 = {0 , 1};
    Point c2 = {1 , 0};
    Point c3 = {0 , -1};
    Point c4 = {-1 , 0};
    c1 = p + c1;
    c2 = p + c2;
    c3 = p + c3;
    c4 = p + c4;
    if (getDoorTileAt(c1.x, c1.y).exists || getDoorTileAt(c2.x, c2.y).exists || getDoorTileAt(c3.x, c3.y).exists || getDoorTileAt(c4.x, c4.y).exists) return true;
    return false;
}