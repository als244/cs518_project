CC = gcc
CFLAGS = -O2

HSA_INCLUDE = -I /opt/rocm/include
HSA_LIB = -L /opt/rocm/lib -lhsa-runtime64

EXECS = hsa_memMapTest

all: $(EXECS)

hsa_memMapTest: memMapTest.c
	$(CC) $(CFLAGS) memMapTest.c -o hsa_memMapTest $(HSA_INCLUDE) $(HSA_LIB)

clean:
	rm -f *.o $(EXECS)
