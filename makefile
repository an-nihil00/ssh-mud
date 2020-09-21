CC=gcc
CFLAGS=-I

ODIR=obj
SDIR=src

LIBS=-lssh

_OBJ = main.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: $(SDIR)/%.c
	$(CC) -c -o $@ $< $(CFLAGS)

bin/ssh-mud: $(OBJ)
	$(CC) -o $@ $^ $(LIBS) $(CFLAGS) 
