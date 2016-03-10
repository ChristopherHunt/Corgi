CC = mpic++
LD = mpic++

OBJS = cache.o cache_node.o cache_node_main.o swing_node.o swing_node_main.o leader_node.o leader_node_main.o
EXECS = cache_node_main swing_node_main leader_node_main

all: $(EXECS)

leader_node_main: leader_node.cpp leader_node_main.cpp
	$(CC) leader_node.cpp leader_node_main.cpp -o leader_node_main

swing_node_main: swing_node.cpp swing_node_main.cpp
	$(CC) swing_node.cpp swing_node_main.cpp -o swing_node_main

cache_node_main: cache_node.cpp cache_node_main.cpp
	$(CC) cache_node.cpp cache_node_main.cpp -o cache_node_main

.PHONY: clean

clean:
	rm -f *.o $(EXECS)
