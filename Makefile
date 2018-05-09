OBJS = pong.cpp
OBJ_NAME = pong

all : $(OBJS)
	g++-7 -std=c++1z $(OBJS) -g -w -lSDL2 -lSDL2_ttf -o $(OBJ_NAME)
