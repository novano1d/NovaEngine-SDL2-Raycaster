all:
	g++ -Ofast -Wall -Wextra -Isrc/Include -Lsrc/lib -o main engine.cpp Pathfinding.cpp levelGen.cpp main.cpp -lmingw32 -lSDL2_ttf -lSDL2main -lSDL2 -lSDLfox -static-libstdc++ -march=x86-64