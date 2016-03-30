CC = mpiicc
LD = mpiicc

APP_DIR = app

build:
	make clean
	make -C lib build
	make run

debug:
	make clean
	make -C lib debug
	make run	

run:
	make -C app run

.PHONY: clean

clean:
	make -C lib clean
	make -C app clean
