CC = mpic++
LD = mpic++
UTILS = ../utils
POLICY = ../policy

EXECS = swing_node_main

build: swing_node.cpp swing_node_main.cpp $(UTILS)/network_header.cpp $(POLICY)/quorum.cpp
	$(CC) swing_node.cpp swing_node_main.cpp $(UTILS)/network_header.cpp $(POLICY)/quorum.cpp -o swing_node_main

.PHONY: clean

clean:
	rm -f *.o $(EXECS)
