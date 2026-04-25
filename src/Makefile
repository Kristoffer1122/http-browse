CC = gcc
CFLAGS = -Wall -Wextra -Werror -g -O3
INCLUDES = -Iurl_parse

SRC = main.c url_parse/url_parse.c
OUT = http-browse

build:
	$(CC) $(CFLAGS) $(INCLUDES) -o $(OUT) $(SRC)

clean:
	rm -f $(OUT)
