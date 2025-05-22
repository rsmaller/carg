CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -Werror -Wcast-qual -Wconversion -O3
RM = rm -f

default: all

all: argsexample

argsexample: argsexample.c
	$(CC) -o argsexample argsexample.c

clean:
	$(RM) argsexample argsexample.obj