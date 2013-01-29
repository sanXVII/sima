
CC = gcc
LD = ld

#CFLAGS += -Wall -O2 `sdl-config --cflags`
CFLAGS += -O2 `sdl-config --cflags`

LIBS = -lm `sdl-config --libs` -lSDL_ttf -lGL 



all: sima

sima: main.o wd_of_ants.o wd_gui.o
	$(CC) -o sima main.o wd_of_ants.o wd_gui.o $(CFLAGS) $(LDFLAGS) $(LIBS)

main.o: main.c
	$(CC) $(CFLAGS) -c -o main.o main.c

wd_gui.o: wd_gui.c
	$(CC) $(CFLAGS) -c -o wd_gui.o wd_gui.c

wd_of_ants.o: wd_of_ants.c
	$(CC) $(CFLAGS) -c -o wd_of_ants.o wd_of_ants.c




clean:
	rm -Rf *.o; rm -Rf sima
