default: static-lib shared-lib

test:
	g++ -g3 cassowary-lite.c test-cassowary-lite.c -L . -Wl,-Bstatic -lcassowary -Wl,-Bdynamic
## Note the last -W1,-Bdynamic: we assume that ld links with
## dynamic versions of the libraries it finds by default; THIS
## ASSUMPTION MAY FAIL in *your* build environment.

static-lib:
	g++ -c cassowary-lite.c -o cassowary-lite.o -L . -Wl,-Bstatic -Wl,-whole-archive -lcassowary -Wl,-Bdynamic -Wl,-no-whole-archive
	ar rcs libcassowary-lite.a cassowary-lite.o
	rm cassowary-lite.o

shared-lib:
	g++ -c -fPIC cassowary-lite.c -o cassowary-lite.o -L . -Wl,-Bstatic -Wl,-whole-archive -lcassowary -Wl,-Bdynamic -Wl,-no-whole-archive
	g++ -shared -Wl,-soname,libcassowary-lite.so -o libcassowary-lite.so cassowary-lite.o
	rm cassowary-lite.o
