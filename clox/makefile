IDIR=include
CC=gcc
# CFLAGS=-I$(IDIR)
CFLAGS=-I$(IDIR) -lm -Wall -Wextra -g3 -O0

SDIR=src
ODIR=$(SDIR)/obj

_DEPS=chunk.h compiler.h debug.h line.h memory.h scanner.h value.h vm.h
DEPS=$(patsubst %,$(IDIR)/%,$(_DEPS))

_XTRADEPS=common.h
XTRADEPS=$(patsubst %,$(IDIR)/%,$(_XTRADEPS))

_OBJ=main.o chunk.o compiler.o debug.o line.o memory.o scanner.o value.o vm.o
OBJ=$(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: $(SDIR)/%.c $(DEPS) $(XTRADEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

clox: $(OBJ) $(XTRADEPS)
	$(CC) -o $@ $^ $(CFLAGS)

run: clox
	./clox

.PHONY: clean run

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~
