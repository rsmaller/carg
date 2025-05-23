CFLAGS = 	-Wall -Wextra -Wpedantic -Werror -Walloca -Wcast-qual -Wconversion \
			-Wsign-conversion -Wshadow -Wfloat-equal -Wundef \
			-Wredundant-decls -Wwrite-strings -Wlogical-op -Wduplicated-cond \
			-Wduplicated-branches -Wjump-misses-init -Wstrict-overflow -Wvla \
			-Winline -Wformat-y2k -Wformat-zero-length -Wmissing-format-attribute -Wuninitialized -O3
SFLAGS = -fsanitize=address -g3
RM = rm -f

default: all

all: argsexample

argsexample: argsexample.c
	$(CC) $(CFLAGS) -o argsexample argsexample.c

sanitized: argsexample.c
	$(CC) $(CFLAGS) $(SANFLAGS) -o argsexample argsexample.c

clean:
	$(RM) argsexample argsexample.obj