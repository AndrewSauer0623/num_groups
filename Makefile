CC = clang
CFLAGS = -Wall -Wextra -g

all: num_groups

groups: num_groups.o
	$(CC) $(CFLAGS) -o num_groups num_groups.o

groups.o: groups.c
	$(CC) $(CFLAGS) -c num_groups.c

clean:
	rm -f groups groups.o
