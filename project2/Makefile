CC=gcc
CFLAGS=-I.
DEPS=mm.h
OBJ=mm.o
LIBS=-pthread

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

mm: $(OBJ)
	gcc -o $@ $^ $(CFLAGS) $(LIBS)

clean:
	rm *.o mm
