CC=gcc
CFLAGS = -g -Wall

all: main

main: DataLink.h AppLayer.h main.c Alarm.h
	$(CC) $(CFLAGS) main.c AppLayer.c DataLink.c Alarm.c -o t1 -lm

clean:
	rm -f t1
