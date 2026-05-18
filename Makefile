IDIR =./include
CC=gcc
CFLAGS=-I$(IDIR)

ODIR=obj
LDIR =./lib

LIBS=-lm

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

install: imgtoff.o
	$(CC) -o imgtoff imgtoff.o $(LIBS)

.PHONY: clean

clean:
	rm -f ./*.o *~ core $(INCDIR)/*~
