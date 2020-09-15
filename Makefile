CC = g++
CFLAGS = -std=c++11 -lglfw -lGLEW -lGL
SRC = src/main.cpp

# Default target : dependencies
all: bin invaders

# @ macro evaluates to current target
bin:
	mkdir -p $@

invaders:
	$(CC) $(CFLAGS) $(SRC) -o ./bin/$@
