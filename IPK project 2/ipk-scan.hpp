/* IPK 2019 - Scanner síťových služeb
 * Marian Kapisinsky, xkapis00
 * 21.04.2019
 * */

#ifndef IPK_SCAN_H
#define IPK_SCAN_H

#include <iostream>
#include <vector>

#include <arpa/inet.h> //INET_ADDRSTRLEN
#include <net/if.h> //IFNAMSIZ

#define TCP_DATAGRAM_SIZE 40
#define UDP_DATAGRAM_SIZE 28

using namespace std;

typedef struct args_s {

	//Device name
	char ifr[IFNAMSIZ];
	
	//List of TCP ports
	vector<string> pt;
	
	//List of UDP ports
	vector<string> pu;
	
	//Source address
	char src_ip[INET_ADDRSTRLEN];

	//Destination address
	char dst_ip[INET_ADDRSTRLEN];

	//Destinatopm address
	char dst_hostname[256];
	
} args_t;

/* Functions */ 

//Validates port numbers and handles port range arguments
void check_optarg(args_t *args, string ports);

//Parse arguments from command line and fills arguments structure
args_t parse_args(int argc, char **argv);

//Used for breaking pcap_next loop after 30 second time limit when no packets are captured
void timeout(int signum);

#endif //IPK_SCAN_H