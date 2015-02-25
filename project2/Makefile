CC=gcc
CFLAGS=-I.
DEPS=mm.h
OBJ=mm.o
EXE=mm
LIBS=-pthread

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(EXE): $(OBJ)
	gcc -o $@ $^ $(CFLAGS) $(LIBS)

clean:
	rm *.o $(EXE) strace-output.txt

strace:
	strace -o strace-output.txt ./$(EXE)
