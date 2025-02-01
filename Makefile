CFLAGS=-Wall -Wextra -Wpedantic

piano: piano.o
	cc $(CFLAGS) -lm -lao -o piano piano.o

install: piano
	install -o root -g root piano /usr/bin/piano

.PHONY: clean
clean:
	rm -f piano
	rm -f *.o
