CC = mpic++
LD = mpic++

EXECS = cache_node_main swing_node_main leader_node_main job_node_main

all: $(EXECS)

leader_node_main: leader_node.cpp leader_node_main.cpp network_header.cpp
	$(CC) leader_node.cpp leader_node_main.cpp network_header.cpp -o leader_node_main

swing_node_main: swing_node.cpp swing_node_main.cpp network_header.cpp
	$(CC) swing_node.cpp swing_node_main.cpp network_header.cpp -o swing_node_main

cache_node_main: cache_node.cpp cache_node_main.cpp network_header.cpp
	$(CC) cache_node.cpp cache_node_main.cpp network_header.cpp -o cache_node_main

job_node_main: job_node_main.cpp network_header.cpp
	$(CC) job_node_main.cpp network_header.cpp -o job_node_main

.PHONY: clean

clean:
	rm -f *.o $(EXECS)
