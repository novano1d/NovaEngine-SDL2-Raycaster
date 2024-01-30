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
#include <ctime>
// Implementation of this example
// https://www.roguebasin.com/index.php/Basic_BSP_Dungeon_generation

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
    static const int SIZE = 30; //base size of the map array init (x by x)
private:
    std::vector<std::vector<int>> map;
    std::vector<std::vector<int>> floorMap;
    std::vector<std::vector<int>> ceilingMap;
    std::vector<std::vector<Door>> doorMap;
    std::vector<std::vector<double>> lightMap;
};

//probably move to cpp but im lazy?
struct Room
{
    Point TLcorner; //top left corner of room
    int xS, yS; //xy sizes

    //for use of tree
    Room* left = nullptr;
    Room* right = nullptr;


    static void splitRoom(Room* room)
    {
        std::cout << room->TLcorner.x << " " << room->TLcorner.y << " " << room->xS << " " << room->yS << std::endl; //debug
        Room* left = new Room();
        Room* right = new Room();
        const int idealRoomSize = 7;
        //base case (change for room variation)
        if ((room->xS - 2) <= idealRoomSize || (room->yS - 2) <= idealRoomSize) return;
        bool hv = (rand() % 2) ? true : false;
        double splitPos;
        if (hv) 
        {
            // Horizontal split
            int minHeight = room->yS * 0.3; // 30% of the room height
            int maxHeight = room->yS * 0.7; // 70% of the room height
            splitPos = room->TLcorner.y + minHeight + rand() % (maxHeight - minHeight);
            // Left room (above split)
            left->TLcorner = room->TLcorner;
            left->xS = room->xS;
            left->yS = splitPos - room->TLcorner.y; 
            // Right room (below split)
            right->TLcorner = {room->TLcorner.x, splitPos};
            right->xS = room->xS;
            right->yS = room->TLcorner.y + room->yS - splitPos;
        } 
        else 
        {
            // Vertical split
            int minWidth = room->xS * 0.3; // 30% of the room width
            int maxWidth = room->xS * 0.7; // 70% of the room width
            splitPos = room->TLcorner.x + minWidth + rand() % (maxWidth - minWidth);
            // Left room (left of split)
            left->TLcorner = room->TLcorner;
            left->xS = splitPos - room->TLcorner.x;
            left->yS = room->yS;    
            // Right room (right of split)
            right->TLcorner = {splitPos, room->TLcorner.y};
            right->xS = room->TLcorner.x + room->xS - splitPos;
            right->yS = room->yS;
        }
        // Assign the new rooms to the left and right pointers of the current room
        room->left = left;
        room->right = right;
        splitRoom(left);
        splitRoom(right);
    }
    static void generateRooms(Room* node, std::vector<std::vector<int>>& map, std::vector<std::vector<Door>>& doorMap)
    {
        // Base case: if node is nullptr, return
        if (node == nullptr) 
        {
            return;
        }

        // If this is a leaf node (no children), generate a room
        if (node->left == nullptr && node->right == nullptr) 
        {
            for (int y = node->TLcorner.y; y < node->TLcorner.y + node->yS; ++y) 
            {
                for (int x = node->TLcorner.x; x < node->TLcorner.x + node->xS; ++x) 
                {
                    if (y < map.size() && x < map[y].size()) 
                    {
                        // Check if the current cell is on the boundary of the room
                        if (y == node->TLcorner.y || y == node->TLcorner.y + node->yS - 1 ||
                            x == node->TLcorner.x || x == node->TLcorner.x + node->xS - 1) 
                        {
                            map[y][x] = 1; // Set boundary cells to 1 (wall)
                        } 
                        else 
                        {
                            map[y][x] = 0; // Set inner cells to 0 (empty space)
                        }
                    }
                }
            }
        } 
        else 
        {
            // If not a leaf, recursively visit children
            generateRooms(node->left, map, doorMap);
            generateRooms(node->right, map, doorMap);
        }
    }

    static void connectRooms(Room* parent, std::vector<std::vector<int>>& map)
    {
        if (parent == nullptr || parent->left == nullptr || parent->right == nullptr) {
            return;
        }
    
        // Find the center points of the left and right rooms
        Point centerLeft = {parent->left->TLcorner.x + parent->left->xS / 2, parent->left->TLcorner.y + parent->left->yS / 2};
        Point centerRight = {parent->right->TLcorner.x + parent->right->xS / 2, parent->right->TLcorner.y + parent->right->yS / 2};
    
        // Draw a corridor connecting these points
        // You can choose to draw a straight line or an L-shaped corridor
        // For simplicity, here's an example of a straight line
        for (int y = std::min(centerLeft.y, centerRight.y); y <= std::max(centerLeft.y, centerRight.y); ++y) {
            for (int x = std::min(centerLeft.x, centerRight.x); x <= std::max(centerLeft.x, centerRight.x); ++x) {
                map[y][x] = 0; // Set to 0 to represent an empty space (corridor)
            }
        }
    
        // Recursively connect child rooms
        connectRooms(parent->left, map);
        connectRooms(parent->right, map);
    }

};
#endif