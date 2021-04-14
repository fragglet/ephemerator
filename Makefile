
PREFIX = /usr/local
CFLAGS = -Wall -fPIC

ephemerator.so : ephemerator.o
	$(CC) -shared $(CFLAGS) $(LDFLAGS) $< -o $@

ephemerator.o : ephemerator.c
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -f ephemerator.so ephemerator.o

install: ephemerator.so
	install ephemerate $(PREFIX)/bin
	install ephemerator.so $(PREFIX)/lib

