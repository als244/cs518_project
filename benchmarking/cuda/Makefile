CC = gcc
CFLAGS = -O2

CUDA_HEADER = /usr/local/cuda/include
export C_INCLUDE_PATH=$(CUDA_HEADER)

EXECS = memMapTest

all: $(EXECS)

memMapTest: memMapTest.c
	$(CC) $(CFLAGS) memMapTest.c -o memMapTest -lcuda

clean:
	rm -f *.o $(EXECS)
