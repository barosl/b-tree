NAME = b-tree
OBJS = main.o

CFLAGS = -Wall -Werror -g

.PHONY: clean

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $+

clean:
	rm -f $(NAME) *.o
