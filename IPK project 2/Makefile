CC=g++
CFLAGS=-std=c++11 -Werror -Wall

all: ipk-scan

interface.o: interface.cpp interface.hpp
	$(CC) $(CFLAGS) -c interface.cpp -o interface.o

net.o: net.cpp net.hpp
	$(CC) $(CFLAGS) -c net.cpp -o net.o

ipk-scan: ipk-scan.cpp interface.o net.o
	$(CC) $(CFLAGS) ipk-scan.cpp interface.o net.o -o ipk-scan -lpcap

tar:
	tar -cvf xkapis00.tar *

clean: 
	rm -rf ipk-scan interface.o net.o xkapis00.tar

