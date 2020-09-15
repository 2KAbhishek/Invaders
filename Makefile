CC = g++
CFLAGS = -std=c++11 -lglfw -lGLEW -lGL
SRC = main.cpp

# Default target : dependencies
all: invaders

# @ macro evaluates to current target
invaders:
	$(CC) $(CFLAGS) $(SRC) -o $@
