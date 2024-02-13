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
#define SKY 0xFFFFF
#include "levelGen.hpp"

Room* Room::generateBasicRoom()
{
   Room* out = new Room();
   const int SIZE = 10;
   for (int i = 0; i < SIZE; i++)
   {
       out->map.push_back(std::vector<int>(SIZE, 0));
   }
   //floor map
   for (int i = 0; i < SIZE; i++)
   {
       out->floorMap.push_back(std::vector<int>(SIZE, 1));
   }
   //ceil map
   for (int i = 0; i < SIZE; i++)
   {
       out->ceilingMap.push_back(std::vector<int>(SIZE, 1));
   }
   //door map
   for (int i = 0; i < SIZE; i++)
   {
       out->doorMap.push_back(std::vector<Door>(SIZE, {0}));
   }
   //light map
   for (int i = 0; i < SIZE; i++)
   {
      //double tem = ((int)((getRandomFloat(0.1f, 1.0f)) * 10) % 10 * 0.1); //light value sensitive to weird floating point values above 0.9
      out->lightMap.push_back(std::vector<double>(SIZE, 1));
      //std::cout << tem << std::endl;
   }
   out->xS = SIZE;
   out->yS = SIZE;
   return out;
}

void levelGen::generateMap()
{
//start:
   map.clear();
   floorMap.clear();
   ceilingMap.clear();
   doorMap.clear();
   lightMap.clear();
   srand(time(nullptr));
   Room* root = new Room();
   root->xS = SIZE;
   root->yS = SIZE;
   //Room::splitRoom(root);
   //Main Level Init (hollow square)
   for (int i = 0; i < SIZE; i++)
   {
      map.push_back(std::vector<int>(SIZE, 1));
   }
   //floor map
   for (int i = 0; i < SIZE; i++)
   {
      floorMap.push_back(std::vector<int>(SIZE, 1));
   }
   //ceil map
   for (int i = 0; i < SIZE; i++)
   {
      ceilingMap.push_back(std::vector<int>(SIZE, 1));
   }
   //door map
   for (int i = 0; i < SIZE; i++)
   {
      doorMap.push_back(std::vector<Door>(SIZE, {0}));
   }
   //light map
   for (int i = 0; i < SIZE; i++)
   {
      lightMap.push_back(std::vector<double>(SIZE, 1));
   }
   Room::generateRooms(map, ceilingMap, floorMap, doorMap, lightMap);
   //if (map[1][1]) goto start;
   //fill any empty edges
   for (int i = 0; i < SIZE; ++i) 
   {
       map[0][i] = 1; // Top edge
       map[SIZE - 1][i] = 1; // Bottom edge
   }
   for (int i = 0; i < SIZE; ++i) 
   {
       map[i][0] = 1; // Left edge
       map[i][SIZE - 1] = 1; // Right edge
   }
   
}