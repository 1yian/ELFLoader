CC = gcc
CFLAGS = -O0 -static
LDFLAGS = -ldl

all: apager dpager hpager testing hpagermore

apager:
	$(CC) -o apager pagers/apager.c pagers/load.c pagers/main.c

dpager:
	$(CC) -o dpager pagers/dpager.c pagers/load.c pagers/main.c

hpager:
	$(CC) -o hpager pagers/hpager.c pagers/load.c pagers/main.c

hpagermore:
	$(CC) -o hpagermore pagers/hpager3.c pagers/load.c pagers/main.c

testing:
	$(CC) $(CFLAGS) -o tests/seq_access tests/seq_access.c
	$(CC) $(CFLAGS) -o tests/sparse_access tests/sparse_access.c
	$(CC) $(CFLAGS) -o tests/sparse_seq_access tests/sparse_seq_access.c
	$(CC) $(CFLAGS) -o tests/single_access tests/single_access.c
	$(CC) $(CFLAGS) -o tests/malloc tests/malloc.c
	$(CC) $(CFLAGS) -o tests/io tests/io.c

clean:
	rm -f apager dpager hpager hpagermore libmymalloc.so
	rm -f tests/seq_access tests/sparse_access tests/sparse_seq_access tests/io tests/malloc tests/single_access
