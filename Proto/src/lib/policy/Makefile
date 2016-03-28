CC = mpic++
LD = mpic++

EXECS = main

build:
	make clean && make quorum

quorum:
	$(CC) quorum.cpp main.cpp -o main

.PHONY: clean

clean:
	rm -f *.o $(EXECS)
