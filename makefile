CC = clang

SDIR = src
IDIR = include
ODIR = obj

_OBJ = main.o gamebasis.o gamewin.o gamelogic.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

CFLAGS = -I$(IDIR) -Wall -Wextra -g
LIBS = -lncurses -pthread

all: tron

$(ODIR)/%.o: $(SDIR)/%.c
	@mkdir -p $(@D)
	$(CC) -c -o $@ $< $(CFLAGS)

tron: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o core
