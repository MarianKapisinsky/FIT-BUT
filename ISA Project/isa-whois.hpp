/* ISA 2019 - Whois tazatel
 * Marian Kapisinsky, xkapis00
 * 18.11.2019
 * */

#ifndef ISA_WHOIS_H
#define ISA_WHOIS_H

#include "isa-tazatel.hpp"
#include <cstring>
#include <cstdlib>

#include <netinet/in.h>
#include <arpa/inet.h>

#define WHOIS_PORT 43

//Sends the WHOIS query
string sendQuery(opts_t opts, struct sockaddr *addr, string query);

//Parses and prints the JSON response from the geolocation web api
void parseJson(string json);

//Assembles the address of WHOIS server, creates the query, calls the sendQuery()
//and parses and filters the WHOIs servers response
//supports whois.ripe.net and whois.nic.cz
//for any other just prints the whole response without comments and empty lines
void whois(opts_t opts);

//Sends HTTP GET request to ipstack.com api server and reads, parses and prints the response using parseJSON() 
void getGeolocation(opts_t opts);

#endif //ISA_WHOIS_H