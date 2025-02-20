PROJECT_NAME := app

CC := gcc
DBGFLAGS := -g -fsanitize=address
LINKERFLAGS := -lncurses
CFLAGS := $(DBGFLAGS) $(LINKERFLAGS) --std=c99 -Wall -Werror

.PHONY: clean

$(PROJECT_NAME): main.c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm $(PROJECT_NAME)
