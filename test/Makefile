CC = gcc
CFLAGS = -Wall -Werror

CDFLAGS = -pg
CRFLAGS = -O3

.PHONY: all debug release
all: pattern_matcher

debug: CFLAGS += $(CDFLAGS)
debug: pattern_matcher
release: CFLAGS += $(CRFLAGS)
release: pattern_matcher

pattern_matcher: pattern_matcher.o pattern.o
	$(CC) $(CFLAGS) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

pattern_matcher.o: pattern_matcher.c
pattern.o: pattern.c

pattern.c: pattern.regex
	../regex2c <pattern.regex >pattern.c

clean:
	rm -f *.o *.out pattern.c pattern_matcher
