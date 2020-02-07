/* ISA 2019 - Whois tazatel
 * Marian Kapisinsky, xkapis00
 * 18.11.2019
 * */

#ifndef ISA_DNS_H
#define ISA_DNS_H

#include <string>
#include <cstring>
#include <cstdlib>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <arpa/nameser.h>
#include <netdb.h>
#include <resolv.h>

#include "isa-tazatel.hpp"

#define LISTSIZE 7

using namespace std;

class DNSResolv {

    public:

        DNSResolv(opts_t initOpts);

        ~DNSResolv(void);

        //Send DNS queries for given records (A, AAAA, MX, CNAME, NS, SOA, PTR), parse response and print the info
        void DNSResolvRun(void);

        string getIPAddr();

        string getHostname();

    private:

        opts_t opts;

        struct __res_state res;

        u_char response[NS_PACKETSZ];

        int responseLen;

        ns_msg msg;

        ns_rr rr;

        string firstIPAddr;

        string firstIPv6Addr;

        string hostname;

        //If -d option is set query the given DNS server, otherwise use DNS resolv.conf
        void ResInit(void);

        void ResSendQuery(int nsType); 

        void ResParseResponse(int nsType);

        void ResPrintInfo();      
};

#endif //ISA_DNS_H
