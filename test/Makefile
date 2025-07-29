CC = gcc
CFLAGS = -Wall --debug

.PHONY: all
all: pattern_matcher

pattern_matcher: pattern_matcher.o pattern.o
	$(CC) $(CFLAGS) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

pattern_matcher.o: pattern_matcher.c
pattern.o: pattern.c

pattern.c: pattern.regex
	cat pattern.regex | tr -d '\n' | ../regex2c >pattern.c

clean:
	rm -f *.o pattern.c pattern_matcher
