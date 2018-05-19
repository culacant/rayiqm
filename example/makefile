CC = gcc.exe
CFLAGS=-I./include -ggdb
LFLAGS=-L./lib -lraylibdll

main: iqmexample.c iqm
	$(CC) -o iqm.exe iqmexample.c iqm.o $(CFLAGS) $(LFLAGS)
iqm: iqm.c
	$(CC) -c -o iqm.o iqm.c $(CFLAGS) $(LFLAGS)
