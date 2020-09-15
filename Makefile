CC = g++
CFLAGS = -std=c++11 -lglfw -lGLEW -lGL
SRC = src/main.cpp

# Default target : dependencies
all: invaders

# @ macro evaluates to current target
invaders: bin
	$(CC) $(CFLAGS) $(SRC) -o ./bin/$@

bin:
	mkdir -p $@

