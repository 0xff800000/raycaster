CC = g++
CFLAGS = -W -Wall -O2 -std=c++20 -g 
LDFLAGS = -lGL -lGLU -lglut
EXEC = raycast

all: $(EXEC)

$(EXEC): main.cpp
	$(CC) -o $(EXEC) $^ $(CFLAGS) $(LDFLAGS)

clean:
	rm $(EXEC)
