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
#ifndef PATHFINDING_HPP
#define PATHFINDING_HPP
#include "engine.hpp"
// followed tutorial at https://dev.to/jansonsa/a-star-a-path-finding-c-4a4h
// to implement
struct Node
{
    Point pos;
    Point parent;
    float gCost;
    float hCost;
    float fCost;
};
inline bool operator<(const Node& lhs, const Node& rhs)
{
    return lhs.fCost < rhs.fCost;
}
class Pathfinder
{
public:
    static void setMap(Map* ptr) { map = ptr;
        xmax = map->xSize();
        ymax = map->ySize(); };
    static bool isValid(int x, int y);
    static bool isDest(int x, int y, Node dest);
    static double calculateH(int x, int y, Node dest);
    static std::vector<Node> aStar(Node player, Node dest);
    static std::vector<Node> makePath(Map map, Node dest);
private:
    inline static Map* map = nullptr;
    static int xmax, ymax;
};


#endif