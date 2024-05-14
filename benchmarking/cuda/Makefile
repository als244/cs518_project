CC = gcc
CFLAGS = -O2

CUDA_INCLUDE = -I /usr/local/cuda/include
### Default paths that system reads from, 
### but being explicit for commonality across backends
CUDA_LIB = -L /usr/local/cuda/lib -lcuda

EXECS = cuda_memMapTest

all: $(EXECS)

cuda_memMapTest: memMapTest.c
	$(CC) $(CFLAGS) memMapTest.c -o cuda_memMapTest $(CUDA_INCLUDE) $(CUDA_LIB)

clean:
	rm -f *.o $(EXECS)

