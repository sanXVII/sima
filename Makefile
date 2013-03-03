
CC = gcc
LD = ld

#CFLAGS += -Wall -O2 `sdl-config --cflags`
CFLAGS += -O2 `sdl-config --cflags`

LIBS = -lm `sdl-config --libs` -lSDL_ttf -lGL -lGLU



all: sima

sima: main.o wd_of_ants.o wd_gui.o sim_drv.o ant_furer.o astar.o
	$(CC) -o sima main.o wd_of_ants.o wd_gui.o sim_drv.o ant_furer.o astar.o $(CFLAGS) $(LDFLAGS) $(LIBS)

main.o: main.c
	$(CC) $(CFLAGS) -c -o main.o main.c

wd_gui.o: wd_gui.c
	$(CC) $(CFLAGS) -c -o wd_gui.o wd_gui.c

wd_of_ants.o: wd_of_ants.c
	$(CC) $(CFLAGS) -c -o wd_of_ants.o wd_of_ants.c

sim_drv.o: sim_drv.c
	$(CC) $(CFLAGS) -c -o sim_drv.o sim_drv.c

ant_furer.o: ant_furer.c
	$(CC) $(CFLAGS) -c -o ant_furer.o ant_furer.c

astar.o: astar.c
	$(CC) $(CFLAGS) -c -o astar.o astar.c


clean:
	rm -Rf *.o; rm -Rf sima
