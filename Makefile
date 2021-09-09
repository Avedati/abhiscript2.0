CC=gcc
CFLAGS=-std=c11 -Wall -g
TARGETS=src/*.c
OUT=bin/abhiscript2.0
ASAN_CFLAGS=-O1 -fsanitize=address -fno-omit-frame-pointer -fno-optimize-sibling-calls -g

all:
	$(CC) $(CFLAGS) $(TARGETS) -o $(OUT)

asan:
	$(CC) $(CFLAGS) $(ASAN_CFLAGS) $(TARGETS) -o $(OUT)

clean:
	rm -rf build/*
	rm -f $(OUT)

test:
	$(MAKE) all
	./$(OUT)
	$(MAKE) clean
