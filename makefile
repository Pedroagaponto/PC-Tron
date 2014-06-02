SDIR = src
IDIR = include
CC = clang
CFLAGS = -I$(IDIR) -Wall -Wextra -g

ODIR = obj

LIBS = -lncurses

_OBJ = main.o 
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: $(SDIR)/%.c
	$(CC) -c -o $@ $< $(CFLAGS)

tron: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ 
