NAME = regex2c
CC = gcc
CFLAGS = -Wall --debug

.PHONY: all debug clean
all: $(NAME)

$(NAME): regex2c.o ast.o automaton.o common.o
	$(CC) $(CFLAGS) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(NAME).o: $(NAME).c ast.h automaton.h common.h
ast.o: ast.c ast.h common.h
automaton.o: automaton.c automaton.h common.h
common.o: common.c common.h

clean:
	rm *.o $(NAME)
