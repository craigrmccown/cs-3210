CFLAGS := -Wall $(shell pkg-config fuse --cflags)
LDFLAGS := $(shell pkg-config fuse --libs)

targets = fuse_hello_world

all: $(targets)

fuse_hello_world: fuse_hello_world.c
#	gcc -Wall fuse_hello_world.c  `pkg-config fuse --cflags --libs` -o fuse_hello_world
	$(CC) $< $(CFLAGS) $(LDFLAGS) -o $@
#	$(CC) $(CFLAGS) $(LDFLAGS) -lulockmgr $< -o $@

clean:
	rm -f *.o
	rm -f $(targets)
