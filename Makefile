CFLAGS = -Wall
SOURCES = example.c libmpdclient.c

example: $(SOURCES)

clean:
	rm -f *.o example
