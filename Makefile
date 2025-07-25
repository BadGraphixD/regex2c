NAME = regex2c
CC = gcc
CFLAGS = -Wall --debug

.PHONY: all debug clean
all: $(NAME)

$(NAME): regex2c.c
	$(CC) $(CFLAGS) $? -o out/$@

clean:
	rm $(NAME)
