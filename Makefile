all:
g++ -Isrc/include -Lsrc/lib -o main main.cpp -lmingw32 -lSDL2main -lSDL2 -lSDL2_image
g++ -Isrc/include -Lsrc/lib -o game game.cpp -lmingw32 -lSDL2main -lSDL2 -lSDL2_image