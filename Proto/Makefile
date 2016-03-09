CC = mpic++
LD = mpic++

OBJS = cache.o cache_node.o cache_node_main.o swing_node.o swing_node_main.o
EXECS = cache_node_main swing_node_main

all: $(EXECS)

cache_node_main: cache_node.cpp cache_node_main.cpp
	$(CC) cache_node.cpp cache_node_main.cpp -o cache_node_main

swing_node_main: swing_node.cpp swing_node_main.cpp
	$(CC) swing_node.cpp swing_node_main.cpp -o swing_node_main

.PHONY: clean

clean:
	rm *.o $(OBJS) $(EXECS)
