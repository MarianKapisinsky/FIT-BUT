/* ISA 2019 - Whois tazatel
 * Marian Kapisinsky, xkapis00
 * 18.11.2019
 * */

#ifndef ISA_TAZATEL_H
#define ISA_TAZATEL_H

#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <arpa/nameser.h>
#include <netdb.h>

using namespace std;

typedef struct {

    bool qHostnameFlag;

    bool qIPFlag;

    bool qIPv6Flag;

    bool wHostnameFlag;

    bool wIPFlag;

    bool wIPv6Flag;

    bool dIPFlag;

    //Queried hostname
    string qHostname;

    //Queried IPv4 address
    string qIP;

    //Queried IPv6 address
    string qIPv6;

    //Queried WHOIS server hostname
    string wHostname;

    //Queried WHOIS server IPv4 address
    string wIP;

    //Queried WHOIS server IPv6 address
    string wIPv6;

    //Queried DNS server IPv4 address
    string dIP;

    //Queried DNS server IPv6 address
    //char dIPv6[INET6_ADDRSTRLEN];
	
} opts_t;

/* Functions */

//Validates the option's argument
int checkOptarg(char *optarg);

//Parses options from command line to arguments structure
opts_t parseOpts(int argc, char **argv);

//Creates reverse IP for DNS PTR query
string reverseIP(int addressFamily, string ip);

#endif //ISA_TAZATEL_H