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
#ifndef LEVELGEN_HPP
#define LEVELGEN_HPP
#include "engine.hpp"
#include <vector>
#include <stack>
#include <utility>
#include <cstdlib>
#include <random>
#include <ctime>

class levelGen
{
public:
    levelGen() { generateMap(); };
    void generateMap();
    // Getters for the maps
    std::vector<std::vector<int>> getMap() { return map; }
    std::vector<std::vector<int>> getFloorMap() { return floorMap; }
    std::vector<std::vector<int>> getCeilMap() { return ceilingMap; }
    std::vector<std::vector<Door>> getDoorMap() { return doorMap; }
    std::vector<std::vector<double>> getLightMap() { return lightMap; }
    static const int SIZE = 40; //base size of the map array init (x by x)
private:
    std::vector<std::vector<int>> map;
    std::vector<std::vector<int>> floorMap;
    std::vector<std::vector<int>> ceilingMap;
    std::vector<std::vector<Door>> doorMap;
    std::vector<std::vector<double>> lightMap;
};

struct Room
{
    std::vector<std::vector<int>> map;
    std::vector<std::vector<int>> floorMap;
    std::vector<std::vector<int>> ceilingMap;
    std::vector<std::vector<Door>> doorMap;
    std::vector<std::vector<double>> lightMap;
    int xS, yS; //xy sizes

    //for use of tree
    Room* left = nullptr;
    Room* right = nullptr;
    //could have a hidden node for secrets maybe

    static float getRandomFloat(float min, float max) {
        // Random engine and distribution
        std::random_device rd;  // Obtain a random number from hardware
        std::mt19937 eng(rd()); // Seed the generator
        std::uniform_real_distribution<> distr(min, max); // Define the range
        return distr(eng);
    }

    static Room* generateBasicRoom();

    static void generateCorridor(std::vector<std::vector<int>>& map, Point& curpos, int lastDirection = -1) 
    {
        
        std::vector<Point> directions = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
    }


    static void generateRooms(std::vector<std::vector<int>>& map, std::vector<std::vector<int>>& floorMap, std::vector<std::vector<int>>& ceilingMap, std::vector<std::vector<Door>>& doorMap, std::vector<std::vector<double>>& lightMap)
    {
        //populate stack with rooms randomly and create a tree... tree traversal will be used to assing rooms (halls between all rooms from random walk)
        std::stack<Room*> rooms;
        Point curpos = {1, 1};
        rooms.push(generateBasicRoom());
        Room* test = rooms.top();
        while (!rooms.empty())
        {
            //place room
            for (int x = 0; x < test->xS; x++)
            {
                for (int y = 0; y < test->yS; y++)
                {
                    map[curpos.y + y][curpos.x + x] = test->map[y][x];
                    floorMap[curpos.y + y][curpos.x + x] = test->floorMap[y][x];
                    ceilingMap[curpos.y + y][curpos.x + x] = test->ceilingMap[y][x];
                    doorMap[curpos.y + y][curpos.x + x] = test->doorMap[y][x];
                    lightMap[curpos.y + y][curpos.x + x] = test->lightMap[y][x];
                }
            }
            curpos.x += test->xS;
            curpos.y += test->yS/2;
            rooms.pop();
            //create corridor
            generateCorridor(map, curpos);
        }
    }
};
#endif