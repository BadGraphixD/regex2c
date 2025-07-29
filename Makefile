CC = gcc
CFLAGS = -Wall --debug

.PHONY: all test clean
all: regex2c

regex2c: regex2c.o regex_parser.o ast2automaton.o automaton2c.o ast.o automaton.o common.o
	$(CC) $(CFLAGS) $^ -o $@

pattern_matcher: pattern_matcher.o pattern.o
	$(CC) $(CFLAGS) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

regex2c.o: regex2c.c regex_parser.h ast2automaton.h automaton2c.h

regex_parser.o: regex_parser.c regex_parser.h ast.h common.h
ast2automaton.o: ast2automaton.c ast2automaton.h ast.h automaton.h
automaton2c.o: automaton2c.c automaton2c.h automaton.h

ast.o: ast.c ast.h common.h
automaton.o: automaton.c automaton.h common.h
common.o: common.c common.h

pattern_matcher.o: pattern_matcher.c
pattern.o: pattern.c

test: regex2c
	cd test && make
	echo "test/pattern_matcher has been generated"

clean:
	rm -f *.o regex2c
	cd test && make clean
