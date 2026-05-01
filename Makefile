CC = gcc
CFLAGS = -Wall -Wextra -Werror -g -O3
INCLUDES = -lssl -lcrypto

SRC = $(shell find . -name '*.c')
OUT = ./bin/http-browse

build:
	mkdir -p bin
	$(CC) $(CFLAGS) $(INCLUDES) -o $(OUT) $(SRC)

clean:
	rm -f $(OUT)
