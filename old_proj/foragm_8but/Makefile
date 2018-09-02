IDIR = ../include
LDIR = source
ODIR = ../objects

CC=gcc
CFLAGS=-pthread -I$(IDIR)

_DEPS = RPio.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))
_OBJ = RPio.o RPio_midiuart.o forgan.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: $(LDIR)/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

forgan:  $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~


