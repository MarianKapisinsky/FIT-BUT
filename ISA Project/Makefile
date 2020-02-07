CC=g++
CCFLAGS=-std=c++11 -Werror -Wall

all: isa-tazatel

isa-dns.o: isa-dns.cpp isa-dns.hpp
	$(CC) $(CCFLAGS) -c isa-dns.cpp -o isa-dns.o

isa-whois.o: isa-whois.cpp isa-whois.hpp
	$(CC) $(CCFLAGS) -c isa-whois.cpp -o isa-whois.o	

isa-tazatel: isa-tazatel.cpp isa-tazatel.hpp isa-dns.o isa-whois.o
	$(CC) $(CCFLAGS) isa-tazatel.cpp isa-dns.o isa-whois.o -o isa-tazatel -lresolv

tar:
	tar -cvf xkapis00.tar *

clean:
	rm -rf isa-tazatel isa-dns.o isa-whois.o xkapis00.tar