CC=gcc
CFLAGS=-ltag_c

cd-merge: cd-merge.o
	$(CC) -o cd-merge cd-merge.o $(CFLAGS)
