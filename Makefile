
CC = gcc
LD = ld

#CFLAGS += -Wall -O2
CFLAGS += -O2

LIBS = -lm



all: sima

sima: main.o wd_of_ants.o
	$(CC) -o sima main.o wd_of_ants.o $(CFLAGS) $(LDFLAGS) $(LIBS)

main.o: main.c
	$(CC) $(CFLAGS) -c -o main.o main.c

wd_of_ants.o: wd_of_ants.c
	$(CC) $(CFLAGS) -c -o wd_of_ants.o wd_of_ants.c




clean:
	rm -Rf *.o; rm sima
