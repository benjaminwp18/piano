CFLAGS=-Wall -Wextra -Wpedantic

piano: piano.o
	cc $(CFLAGS) -o piano piano.o -lm -lao

install: piano
	install -o root -g root piano /usr/bin/piano

.PHONY: clean
clean:
	rm -f piano
	rm -f *.o
