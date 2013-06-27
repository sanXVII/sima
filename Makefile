
CC = gcc
LD = ld

#CFLAGS += -Wall -O2 `sdl-config --cflags`
CFLAGS += -O3 `sdl-config --cflags` -g

LIBS = -lm `sdl-config --libs` -lSDL_ttf -lGL -lGLU



all: sima rtree_test

sima: main.o wd_of_ants.o wd_gui.o sim_drv.o ant_furer.o astar.o rtree.o wd_free_pix.o
	$(CC) -o sima main.o wd_of_ants.o wd_gui.o sim_drv.o ant_furer.o astar.o rtree.o wd_free_pix.o $(CFLAGS) $(LDFLAGS) $(LIBS)

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

rtree.o: rtree.c
	$(CC) $(CFLAGS) -c -o rtree.o rtree.c

wd_free_pix.o: wd_free_pix.c
	$(CC) $(CFLAGS) -c -o wd_free_pix.o wd_free_pix.c

rtree_test: rtree.o rtree_test.c
	$(CC) rtree_test.c -o rtree_test rtree.o $(CFLAGS) $(LDFLAGS) $(LIBS)

clean:
	rm -Rf *.o; rm -Rf sima rtree_test
