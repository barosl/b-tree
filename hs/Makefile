NAME = b-tree
OBJS = Main.o BTree.o

all: $(NAME)

Main.hs: BTree.o

.SUFFIXES: .hs
.PHONY: clean

$(NAME): $(OBJS)
	ghc -o $@ $+
	strip $@

.hs.o:
	ghc -c $<

clean:
	rm -f $(NAME) *.o *.hi
