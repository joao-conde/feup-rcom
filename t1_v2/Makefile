CC=gcc
CFLAGS = -Wall

all: main

main: DataLink.h AppLayer.h main.c Alarm.h
	$(CC) $(CFLAGS) main.c AppLayer.c DataLink.c Alarm.c -o main -lm

clean:
	rm -f main
