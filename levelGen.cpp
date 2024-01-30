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
#include "levelGen.hpp"

void levelGen::generateMap()
{
   srand(time(nullptr));
   Room* root = new Room();
   root->TLcorner = {0, 0};
   root->xS = SIZE;
   root->yS = SIZE;
   Room::splitRoom(root);
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
   Room::generateRooms(root, map, doorMap);
   Room::connectRooms(root, map);

   /*
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
   */


   /*
   //Maze Test
   std::stack<std::pair<int, int>> stack;
   // Choose a random starting cell that is not on the border
   int x = 1;
   int y = 1;
   map[x][y] = 0;
   stack.push({x, y});

   while (!stack.empty())
   {
      auto [cx, cy] = stack.top();
      stack.pop();
      // Directions: up, right, down, left
      std::vector<std::pair<int, int>> directions = {{-1, 0}, {0, 1}, {1, 0}, {0, -1}, {-1, 0}, {0, 1}, {1, 0}, {0, -1}};
      std::random_shuffle(directions.begin(), directions.end());
      for (auto& [dx, dy] : directions)
      {
         int nx = cx + dx * 2;
         int ny = cy + dy * 2;
         // Check if the new cell is within the map and not on the border
         if (nx > 0 && ny > 0 && nx < SIZE - 1 && ny < SIZE - 1 && map[nx][ny] == 1)
         {
            map[nx][ny] = 0;
            map[cx + dx][cy + dy] = 0; // Remove wall between cells
            stack.push({nx, ny});
            break;
         }
      }
   }
   */
}