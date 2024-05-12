CC = gcc
CFLAGS = -O2

ZE_INCLUDE = -I /usr/include/level_zero
### Default paths that system reads from, 
### but being explicit for commonality across backends
ZE_LIB = -L /usr/lib/x86_64-linux-gnu -lze_loader

EXECS = ze_memMapTest

all: $(EXECS)

ze_memMapTest: memMapTest.c
	$(CC) $(CFLAGS) memMapTest.c -o ze_memMapTest $(ZE_INCLUDE) $(ZE_LIB)

clean:
	rm -f *.o $(EXECS)

