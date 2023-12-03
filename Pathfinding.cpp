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
#include "Pathfinding.hpp"
#include <stack>

void Pathfinder::setMap(Map* m) { map = m; xmax = map->xSize(); ymax = map->ySize(); };

std::vector<Node> Pathfinder::makePath(std::vector<std::vector<Node>> map, Node dest) {
    try {
        std::cout << "Found a path" << std::endl;
        int x = dest.pos.x;
        int y = dest.pos.y;
        std::stack<Node> path;
        std::vector<Node> usablePath;
        while (!(map[x][y].parent.x == x && map[x][y].parent.x == y)
            && map[x][y].pos.x != -1 && map[x][y].pos.y != -1) 
        {
            path.push(map[x][y]);
            int tempX = map[x][y].parent.x;
            int tempY = map[x][y].parent.y;
            x = tempX;
            y = tempY;
        }
        path.push(map[x][y]);
        while (!path.empty()) {
            Node top = path.top();
            path.pop();
            usablePath.emplace_back(top);
        }
        return usablePath;
    }
    catch(const _exception& e){
        std::cout << e.name << std::endl;
    }
}

bool Pathfinder::isValid(int x, int y)
{
    if (map->getTileAt(x,y) == 0) return true;
    if (x < 0 || y < 0 || x >= (xmax) || y >= (ymax)) {
        return false;
    }
    if (map->getTileAt(x,y) > 0) return false;
    if (map->getDoorTileAt(x,y).state == DOOR_CLOSED || map->getDoorTileAt(x,y).state == DOOR_CLOSING) return false;
    return false;
}

bool Pathfinder::isDest(int x, int y, Node dest) {
    if (x == dest.pos.x && y == dest.pos.y) {
        return true;
    }
    return false;
}

double Pathfinder::calculateH(int x, int y, Node dest) {
    double H = (sqrt((x - dest.pos.x)*(x - dest.pos.x)
        + (y - dest.pos.y)*(y - dest.pos.y)));
    return H;
}

std::vector<Node> Pathfinder::aStar(Node player, Node dest) 
{
    std::vector<Node> empty;
    if (isValid(dest.pos.x, dest.pos.y) == false) {
        std::cout << "Destination is an obstacle" << std::endl;
        return empty;
        //Destination is invalid
    }
    if (isDest(player.pos.x, player.pos.y, dest)) {
        std::cout << "You are the destination" << std::endl;
        return empty;
        //You clicked on yourself
    }
    bool closedList[(xmax)][(ymax)];
    //Initialize whole map
    //Node allMap[50][25];
    std::vector<std::vector<Node>> allMap(xmax, std::vector<Node>(ymax));
    for (int x = 0; x < (xmax); x++) {
        for (int y = 0; y < (ymax); y++) {
            allMap[x][y].fCost = std::numeric_limits<float>::max();
            allMap[x][y].gCost = std::numeric_limits<float>::max();
            allMap[x][y].hCost = std::numeric_limits<float>::max();
            allMap[x][y].parent.x = -1;
            allMap[x][y].parent.y = -1;
            allMap[x][y].pos.x = x;
            allMap[x][y].pos.y = y;
            closedList[x][y] = false;
        }
    }
    //Initialize our starting list
    int x = player.pos.x;
    int y = player.pos.y;
    allMap[x][y].fCost = 0.0;
    allMap[x][y].gCost = 0.0;
    allMap[x][y].hCost = 0.0;
    allMap[x][y].parent.x = x;
    allMap[x][y].parent.y = y;
    std::vector<Node> openList;  
    openList.emplace_back(allMap[x][y]);
    bool destinationFound = false;
    while (!openList.empty()&&openList.size()<(xmax)*(ymax)) {
        Node node;
        do {
            //This do-while loop could be replaced with extracting the first
            //element from a set, but you'd have to make the openList a set.
            //To be completely honest, I don't remember the reason why I do
            //it with a vector, but for now it's still an option, although
            //not as good as a set performance wise.
            float temp = std::numeric_limits<float>::max();
            std::vector<Node>::iterator itNode;
            for (std::vector<Node>::iterator it = openList.begin();
                it != openList.end(); it = next(it)) {
                Node n = *it;
                if (n.fCost < temp) {
                    temp = n.fCost;
                    itNode = it;
                }
            }
            node = *itNode;
            openList.erase(itNode);
        } while (isValid(node.pos.x, node.pos.y) == false);
        x = node.pos.x;
        y = node.pos.y;
        closedList[x][y] = true;
        //For each neighbour starting from North-West to South-East
        for (int newX = -1; newX <= 1; newX++) {
            for (int newY = -1; newY <= 1; newY++) {
                double gNew, hNew, fNew;
                if (isValid(x + newX, y + newY)) {
                    if (isDest(x + newX, y + newY, dest))
                    {
                        //Destination found - make path
                        allMap[x + newX][y + newY].parent.x = x;
                        allMap[x + newX][y + newY].parent.y = y;
                        destinationFound = true;
                        return makePath(allMap, dest);
                    }
                    else if (closedList[x + newX][y + newY] == false)
                    {
                        gNew = node.gCost + 1.0;
                        hNew = calculateH(x + newX, y + newY, dest);
                        fNew = gNew + hNew;
                        // Check if this path is better than the one already present
                        if (allMap[x + newX][y + newY].fCost == std::numeric_limits<float>::max() ||
                            allMap[x + newX][y + newY].fCost > fNew)
                        {
                            // Update the details of this neighbour node
                            allMap[x + newX][y + newY].fCost = fNew;
                            allMap[x + newX][y + newY].gCost = gNew;
                            allMap[x + newX][y + newY].hCost = hNew;
                            allMap[x + newX][y + newY].parent.x = x;
                            allMap[x + newX][y + newY].parent.y = y;
                            openList.emplace_back(allMap[x + newX][y + newY]);
                        }
                    }
                }
            }
        }
        }
        if (destinationFound == false) {
            std::cout << "Destination not found" << std::endl;
            return empty;
    }
}