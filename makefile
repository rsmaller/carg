CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -Werror -Wcast-qual -Wconversion -O3
RM = rm -f

default: all

all: args

args: args.c
	$(CC) -o args args.c

clean:
	$(RM) args