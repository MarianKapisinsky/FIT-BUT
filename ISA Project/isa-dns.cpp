/* ISA 2019 - Whois tazatel
 * Marian Kapisinsky, xkapis00
 * 18.11.2019
 * */

#include <iostream>
#include "isa-dns.hpp"

/* Public functions */

DNSResolv::DNSResolv(opts_t initOpts) {

    opts = initOpts;
}

DNSResolv::~DNSResolv(void) {

}

void DNSResolv::DNSResolvRun(void) {

    int nsTypeList[LISTSIZE] = {ns_t_a, ns_t_aaaa, ns_t_mx, ns_t_cname, ns_t_ns, ns_t_soa, ns_t_ptr};

    ResInit();
    
    cout << "=== DNS ===" << endl;

    for (int i = 0; i < LISTSIZE; i++){
        ResSendQuery(nsTypeList[i]);
        ResParseResponse(nsTypeList[i]); 
    }

    res_nclose(&res);
}

string DNSResolv::getIPAddr() {

    return firstIPAddr;
}

string DNSResolv::getHostname() {

    return hostname;
}

/* Private functions */

void DNSResolv::ResInit(void) {

    res_ninit(&res);

    if (opts.dIPFlag) {

        struct in_addr addr;
        inet_aton(opts.dIP.c_str(), &addr);

        res.nscount = 1;
        res.nsaddr_list[0].sin_addr = addr;
        res.nsaddr_list[0].sin_family = AF_INET;
        res.nsaddr_list[0].sin_port = htons(NS_DEFAULTPORT);    
    }    
}

void DNSResolv::ResSendQuery(int nsType) {

    u_char query[NS_PACKETSZ];
    int queryLen = 0;

    if (opts.qHostnameFlag)
        queryLen = res_mkquery( ns_o_query, opts.qHostname.c_str(), ns_c_in, nsType, (u_char *)NULL, 0, (u_char *)NULL, (u_char *)&query, sizeof(query));
    else if (opts.qIPFlag) {

        string ip(opts.qIP);
        queryLen = res_mkquery( ns_o_query, reverseIP(AF_INET, ip).c_str(), ns_c_in, nsType, (u_char *)NULL, 0, (u_char *)NULL, (u_char *)&query, sizeof(query));
    }
    else if (opts.qIPv6Flag) {

        string ipv6(opts.qIPv6);
        queryLen = res_mkquery( ns_o_query, reverseIP(AF_INET6, ipv6).c_str(), ns_c_in, nsType, (u_char *)NULL, 0, (u_char *)NULL, (u_char *)&query, sizeof(query));        
    }
    
    responseLen = res_nsend(&res, (u_char *)&query, queryLen, response, sizeof(response));
    if (responseLen < 0){
        cerr << "No response received!" << endl;
        return;
    }
}

void DNSResolv::ResParseResponse(int nsType) {

    int msgCount;

    bool an = false, ns = false;

    if (ns_initparse(response, responseLen, &msg) < 0) { 
        cerr << "ns_initparse() failed!" << endl;
        return;
    }

    if ((msgCount = ns_msg_count(msg, ns_s_an)) != 0) {
        an = true;
    }
    else if ((msgCount = ns_msg_count(msg, ns_s_ns)) != 0) {
        ns = true;
    }
    else {
        //cerr << "No resource record received!" << endl;
        return;
    }
    
    for (int i = 0; i < msgCount; i++) {

        if (an) {
            if (ns_parserr(&msg, ns_s_an, i, &rr) != 0) {
                cerr << "ns_parserr error!" << endl;
                return;
            }

            if (ns_rr_type(rr) != nsType) continue;

            ResPrintInfo();
        }
        else if (ns && (nsType == ns_t_soa)) {
            if (ns_parserr(&msg, ns_s_ns, i, &rr) != 0) {
                cerr << "ns_parserr error!" << endl;
                return;
            }
            
            ResPrintInfo();
        } 
    }
}

void DNSResolv::ResPrintInfo(void) {

    char buffer[MAXDNAME];
    memset(&buffer, 0, sizeof(buffer));

    int rrType = ns_rr_type(rr);
    u_char *data = (u_char *) ns_rr_rdata(rr);

    int n = 0;

    switch (rrType) {

        case ns_t_a: 

            cout << "A:\t\t" << inet_ntop(AF_INET, data, buffer, sizeof(buffer)) << endl;
            if (firstIPAddr.empty()) firstIPAddr = buffer;
            break;

        case ns_t_aaaa:

            cout << "AAAA:\t\t" << inet_ntop(AF_INET6, data, buffer, sizeof(buffer)) << endl;
            if (firstIPv6Addr.empty()) firstIPv6Addr = buffer;
            break;

        case ns_t_mx:

            if (ns_name_uncompress( ns_msg_base(msg), ns_msg_end(msg), data +2, buffer, MAXDNAME) < 0) {
                cerr << "ns_name_uncompress() failed!" << endl;
                return;
            }
            cout << "MX:\t\t" << buffer << ", preference " << ns_get16(data) << endl;
            break;

        case ns_t_cname:

            if (ns_name_uncompress( ns_msg_base(msg), ns_msg_end(msg), data, buffer, MAXDNAME) < 0) {
                cerr << "ns_name_uncompress() failed!" << endl;
                return;
            }
            cout << "CNAME:\t\t" << buffer << endl;
            break;

        case ns_t_ns:

            if (ns_name_uncompress( ns_msg_base(msg), ns_msg_end(msg), data, buffer, MAXDNAME) < 0) {
                cerr << "ns_name_uncompress() failed!" << endl;
                return;
            }
            cout << "NS:\t\t" << buffer << endl; 
            break;

        case ns_t_soa:

            if ((n = ns_name_uncompress( ns_msg_base(msg), ns_msg_end(msg), data, buffer, MAXDNAME)) < 0) {
                cerr << "ns_name_uncompress() failed!" << endl;
                return;
            }
            cout << "SOA:\t\t" << buffer << endl;

            if ((n = ns_name_uncompress( ns_msg_base(msg), ns_msg_end(msg), data + n, buffer, MAXDNAME)) < 0) {
                cerr << "ns_name_uncompress() failed!" << endl;
                return;
            }
            cout << "admin mail:\t" << buffer << endl;
            break;

        case ns_t_ptr:
            
            if (ns_name_uncompress( ns_msg_base(msg), ns_msg_end(msg), data, buffer, MAXDNAME) < 0) {
                cerr << "ns_name_uncompress() failed!" << endl;
                return;
            }
            if (hostname.empty()) hostname = buffer;
            cout << "PTR:\t\t" << buffer << endl;
            break;
    }
}
