
CFLAGS = -Wall -fPIC

ephemerator.so : ephemerator.o
	$(CC) -shared $(CFLAGS) $(LDFLAGS) $< -o $@

ephemerator.o : ephemerator.c
	$(CC) -c $(CFLAGS) $< -o $@

