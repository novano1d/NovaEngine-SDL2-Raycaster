all:
	g++ -Ofast -Isrc/Include -Lsrc/lib -o main main.cpp engine.cpp -lmingw32 -lSDL2_ttf -lSDL2main -lSDL2 -lSDLfox -static-libstdc++ -march=x86-64 -std=c++2b