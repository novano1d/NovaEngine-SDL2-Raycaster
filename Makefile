all:
	g++ -O3 -Isrc/Include -Lsrc/lib -o main main.cpp engine.cpp -lmingw32 -lSDL2_ttf -lSDL2main -lSDL2 -lSDLfox -static-libstdc++ -march=native