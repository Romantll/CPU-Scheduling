CC = gcc
COPS = -D_GNU_SOURCE -g

all: assign2

assign2: assign2.c pslibrary.h pslibrary.c
        $(CC) $(COPS) -o assign2 assign2.c pslibrary.c
        $(CC) $(COPS) -o runall runall.c pslibrary.c
clean:
        rm runall assign2 *.all