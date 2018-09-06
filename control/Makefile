CC=gcc
CFLAGS=-lasound -lcurses

OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))


control: 
	$(CC) -o$@ $^ $(CFLAGS) $@.c



