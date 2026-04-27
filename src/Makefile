CC = gcc
CFLAGS = -Wall -Wextra -Werror -g -O3
INCLUDES = -lssl -lcrypto

SRC = main.c
OUT = http-browse

build:
	$(CC) $(CFLAGS) $(INCLUDES) -o $(OUT) $(SRC)

clean:
	rm -f $(OUT)
