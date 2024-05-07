CC = gcc
## tell compiler we are compiling HIP for AMD
CFLAGS = -g -fPIC -D__HIP_PLATFORM_AMD__

PROJ_DIR = .

# Backend specific 
HIP_PLUGIN_INTERFACE = $(PROJ_DIR)/backends/hsa/include
# Header + Lib supplied by vendor
HIP_HEADER = /opt/rocm/include
export C_INCLUDE_PATH=$(HIP_HEADER)
HIP_LIB = -L /opt/rocm/lib -lamdhip64 -lhsa-runtime64

# For general layer objects
LAYER_OBJS = client.o system_config.o system_init.o server.o request_handlers.o master.o host.o device.o node.o phys_chunk.o chunks.o obj.o obj_table.o deque.o utils.o
LAYER_INCLUDE = $(PROJ_DIR)/include
LAYER_LIB = -pthread -lm 
#LAYER_EXECS = server

## EXAMPLE HIP BACKEND

## used to access the plugin functions/structs blindly
BACKEND_PLUGIN = $(HIP_PLUGIN_INTERFACE)

## used to build the backend object file
BACKEND_INCLUDE = $(LAYER_INCLUDE) -I $(HIP_PLUGIN_INTERFACE)
BACKEND_SRC = $(HIP_SRC)
BACKEND_LIB = $(HIP_LIB)

## TODO:
##	- would be cleaner to combine into single shared object file
BACKEND_OBJS = driver.o context.o phys_mem.o write.o read.o


## EXECUTABLES TO TEST LAYER
## Demo with hip backend
BACKEND = hsa
DEV_EXECS = example_client


# all: server
all: $(DEV_EXECS)
	rm -f *.o

clean:
	rm -f *.o $(DEV_EXECS)

### IMPORTANT TODOs:
##		- Reorganize header files and includes to be cleaner
##			- Draw out clear dependency charts
##		- Ensure valid seperation between client/server/backend!

# server: server.o $(LAYER_OBJS) $(BACKEND_OBJS)
# 	$(CC) $(CFLAGS) $(OBJS) $(BACKEND_OBJS) -o server $(LAYER_LIB) $(BACKEND_LIB)

example_client: example_client.o $(BACKEND_OBJS) $(LAYER_OBJS) 
	$(CC) $(CFLAGS) example_client.o $(BACKEND_OBJS) $(LAYER_OBJS) -o example_client $(LAYER_LIB) $(BACKEND_LIB)

example_client.o: example_client.c
	$(CC) $(CFLAGS) -c example_client.c -I$(BACKEND_INCLUDE)


## LAYER

## Layer objects calling the backend
## TODO:
##	- fix these headers to do forward declaration for cleaner organization/seperation
client.o: client.c
	$(CC) $(CFLAGS) -c client.c -I$(BACKEND_INCLUDE)

request_handlers.o: request_handlers.c
	$(CC) $(CFLAGS) -c request_handlers.c -I$(BACKEND_INCLUDE)

phys_chunk.o: phys_chunk.c
	$(CC) $(CFLAGS) -c phys_chunk.c -I$(BACKEND_INCLUDE)

device.o: device.c
	$(CC) $(CFLAGS) -c device.c -I$(BACKEND_INCLUDE)

node.o: node.c
	$(CC) $(CFLAGS) -c node.c -I$(BACKEND_INCLUDE)

obj.o: obj.c
	$(CC) $(CFLAGS) -c obj.c -I$(BACKEND_INCLUDE)


## Strictly layer objects
system_config.o: system_config.c
	$(CC) $(CFLAGS) -c system_config.c -I$(LAYER_INCLUDE)

system_init.o: system_init.c
	$(CC) $(CFLAGS) -c system_init.c -I$(LAYER_INCLUDE)

server.o: server.c
	$(CC) $(CFLAGS) -c server.c -I$(LAYER_INCLUDE)

master.o: master.c
	$(CC) $(CFLAGS) -c master.c -I$(LAYER_INCLUDE)

host.o: host.c
	$(CC) $(CFLAGS) -c host.c -I$(LAYER_INCLUDE)

obj_table.o: obj_table.c
	$(CC) $(CFLAGS) -c obj_table.c -I$(LAYER_INCLUDE)

chunks.o: chunks.c
	$(CC) $(CFLAGS) -c chunks.c -I$(LAYER_INCLUDE)

deque.o: deque.c
	$(CC) $(CFLAGS) -c deque.c -I$(LAYER_INCLUDE)

utils.o: utils.c
	$(CC) $(CFLAGS) -c utils.c -I$(LAYER_INCLUDE)



# BACKEND

BACKEND_SRC_DIR = $(PROJ_DIR)/backends/$(BACKEND)

driver.o: $(BACKEND_SRC_DIR)/driver.c
	$(CC) $(CFLAGS) -c $(BACKEND_SRC_DIR)/driver.c -I$(BACKEND_INCLUDE)

context.o: $(BACKEND_SRC_DIR)/context.c
	$(CC) $(CFLAGS) -c $(BACKEND_SRC_DIR)/context.c -I$(BACKEND_INCLUDE)

phys_mem.o: $(BACKEND_SRC_DIR)/phys_mem.c
	$(CC) $(CFLAGS) -c $(BACKEND_SRC_DIR)/phys_mem.c -I$(BACKEND_INCLUDE)

write.o: $(BACKEND_SRC_DIR)/write.c
	$(CC) $(CFLAGS) -c $(BACKEND_SRC_DIR)/write.c -I$(BACKEND_INCLUDE)

read.o: $(BACKEND_SRC_DIR)/read.c
	$(CC) $(CFLAGS) -c $(BACKEND_SRC_DIR)/read.c -I$(BACKEND_INCLUDE)

