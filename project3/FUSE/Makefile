FLAGS := -Wall $(shell pkg-config fuse --cflags) 
LDFLAGS := $(shell pkg-config fuse --libs) -D_FILE_OFFSET_BITS=64 -lcrypto -lssl

targets = rpfs

all: $(targets)
	mkdir -p mountdir

rpfs: rpfs.c
	$(CC) $< $(CFLAGS) $(LDFLAGS) -o $@

clean:
	rm -f *.o
	rm -f $(targets)
	fusermount -u mountdir
	rmdir mountdir
	rm log.txt
	touch log.txt
