CC = gcc
CFLAGS = -Wall -Werror
LD = ld
LIB_TARGET = lib

CDFLAGS = -pg -g
CRFLAGS = -O3

.PHONY: all debug release lib lib_debug lib_release test clean
all: regex2c

debug: CFLAGS += $(CDFLAGS)
debug: LIB_TARGET = lib_debug
debug: regex2c
lib_debug: CFLAGS += $(CDFLAGS)
lib_debug: LIB_TARGET = lib_debug
lib_debug: lib
release: CFLAGS += $(CRFLAGS)
release: LIB_TARGET = lib_release
release: regex2c
lib_release: CFLAGS += $(CRFLAGS)
lib_release: LIB_TARGET = lib_release
lib_release: lib

regex2c: regex2c.o regex_parser.o ast2automaton.o automaton2c.o ast.o automaton.o common.o not_enough_cli/bin/lib.o
	$(CC) $(CFLAGS) $^ -o $@

lib: regex_parser.o ast2automaton.o automaton2c.o ast.o automaton.o common.o
	$(LD) -r $^ -o lib.o

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

not_enough_cli/bin/lib.o:
	@cd not_enough_cli && make $(LIB_TARGET)

test: regex2c
	@cd test && make
	@echo "test/pattern_matcher has been generated"

clean:
	rm -f *.o *.out regex2c
	@cd test && make clean
	@cd not_enough_cli && make clean
