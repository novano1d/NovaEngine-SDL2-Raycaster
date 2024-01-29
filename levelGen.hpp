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
#include <vector>
#include "engine.hpp"

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
private:
    static const int SIZE = 20; //base size of the map array init (x by x)
    std::vector<std::vector<int>> map;
    std::vector<std::vector<int>> floorMap;
    std::vector<std::vector<int>> ceilingMap;
    std::vector<std::vector<Door>> doorMap;
    std::vector<std::vector<double>> lightMap;
};