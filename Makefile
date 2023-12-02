CC=gcc
CFLAGS=-I.
DEPS = main.c
OBJ = main.o
LIBS=-lm

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

main: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

all: main run

clean:
	rm -f *.o main

run:
	./race-server ./race-mid ./race-dumb ./main