/* ISA 2019 - Whois tazatel
 * Marian Kapisinsky, xkapis00
 * 18.11.2019
 * */

#include <iostream>
#include <string>
#include <regex>
#include <vector>
#include <unistd.h>
#include <cstring>
#include <ctype.h>

#include "isa-tazatel.hpp"
#include "isa-whois.hpp"
#include "isa-dns.hpp"

using namespace std;

regex ipv4_r("^(([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5]).){3}([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])$");
regex ipv6_r("^(([0-9A-Fa-f]{1,4}:){7})([0-9A-Fa-f]{1,4})$|(([0-9A-Fa-f]{1,4}:){1,6}:)(([0-9A-Fa-f]{1,4}:){0,4})([0-9A-Fa-f]{1,4})$");

int checkOptarg(char *optarg) {

    if (strcmp(optarg, "-q") == 0 || strcmp(optarg, "-w") == 0 || strcmp(optarg, "-d") == 0) {
        cerr << "Wrong option syntax!" << endl;
        exit(EXIT_FAILURE);
    }

    if (regex_match(optarg, ipv4_r)) {
                
        return 4;
    }
    else if (regex_match(optarg, ipv6_r) || strcmp(optarg, "::1") == 0) {
                
        return 6;
    }
    else {

        return 0;
    }
}

opts_t parseOpts(int argc, char **argv) {

    opts_t opts;

    opts.qHostnameFlag = false;

    opts.qIPFlag = false;

    opts.qIPv6Flag = false;

    opts.wHostnameFlag = false;

    opts.wIPFlag = false;

    opts.wIPv6Flag = false;

    opts.dIPFlag = false;

    int opt, check;

    opterr = 0;

    while ((opt = getopt(argc, argv, "q:w:d:")) != -1) {

        switch (opt) {

            case 'q':

                check = checkOptarg(optarg);

                switch (check) {

                    case 0:
                        opts.qHostname = optarg;
                        opts.qHostnameFlag = true;
                        break;

                    case 4:
                        opts.qIP = optarg;
                        opts.qIPFlag = true;
                        break;

                    case 6:
                        opts.qIPv6 = optarg;
                        opts.qIPv6Flag = true;
                        break;
                } 	
                break;
            
            case 'w':

                check = checkOptarg(optarg);

                switch (check) {

                    case 0:
                        opts.wHostname = optarg;
                        opts.wHostnameFlag = true;
                        break;

                    case 4:
                        opts.wIP = optarg;
                        opts.wIPFlag = true;
                        break;
                    
                    case 6:
                        opts.wIPv6 = optarg;
                        opts.wIPv6Flag = true;
                        break;
                } 	
                break;

            case 'd':

                check = checkOptarg(optarg);

                switch (check) {

                    case 0:
                    case 6:
                        cerr << "Wrong argument -d syntax!" << endl;
                        exit(EXIT_FAILURE);

                    case 4:
                        opts.dIP = optarg;
                        opts.dIPFlag = true;
                        break;

                    //case 6:
                    //  opts.qIPv6 = optarg;
                    //  opts.dIPv6Flag = true;
                    //  break;
                }
                break;

            case '?':
                if (optopt == 'q' || optopt == 'w' || optopt == 'd')
                    cerr << "Option -" << optopt << "requires an argument!" << endl;
                else if (isprint (optopt))
                    cerr << "Unknown option -" << optopt << "!" << endl;
                else
                    cerr << "Unknown option character " << optopt << "!" << endl;
                exit(EXIT_FAILURE);

            default:
                exit(EXIT_SUCCESS);
        }
    }

    return opts;
}

string reverseIP(int addressFamily, string ip) {

        string s = ip;
        string delimiter;
        size_t pos = 0;
        string token;
        string reversed;
        vector<string> v; 

    if (addressFamily == AF_INET) { 

        delimiter = ".";

        while ((pos = s.find(delimiter)) != string::npos) {
            token = s.substr(0, pos);
            v.push_back(token);
            s.erase(0, pos + delimiter.length());
        }
        token = s.substr(0, pos);
        v.push_back(token);

        while (!v.empty()) {
            reversed += v.back() + ".";
            v.pop_back();
        }

        reversed += "in-addr.arpa";
    }
    else if (addressFamily == AF_INET6) {

        string dup = s;
        delimiter = ":";

        while ((pos = dup.find(delimiter)) != string::npos) {
            token = dup.substr(0, pos);
            v.push_back(token);
            dup.erase(0, pos + delimiter.length());
        }
        token = dup.substr(0, pos);
        v.push_back(token);

        reverse(s.begin(), s.end());

        unsigned count = 0;

        for (unsigned i = 0; i < s.length(); ++i) {

            if (s.at(i) != ':') {
                reversed += s.at(i);
                reversed += ".";
                count++;
            }
            else {
                if (s.at(i+1) == ':') {
                    for (unsigned zeros = 0; zeros < 4 * (8 - (v.size() - 1)); ++zeros) reversed += "0.";
                    i++;
                }
                else if (count != 4) {
                    for (unsigned zeros = 0; zeros < 4 - count; ++zeros) reversed += "0.";
                }
                count = 0;
                continue;
            }
        }

        reversed += "ip6.arpa";
    }
    else
        return ip;

    return reversed;
}

int main(int argc, char **argv) {

    opts_t opts = parseOpts(argc, argv); //Parse options

    //Prints help if no options set
    if (argc == 0) {
        cout << "No options set!" << endl;
        cout << "-q\tQueried IP or hostname" << endl;
        cout << "-w\tSpecifies queried WHOIS server (hostname or IP)" << endl;
        cout << "-d\tSpecifies queried DNS server (only IPv4 is supported)" << endl;
        exit(EXIT_SUCCESS);
    }

    //Check if required options were set
    if (!opts.qHostnameFlag && !opts.qIPFlag && !opts.qIPv6Flag) {
        cerr << "Required option -q not set!" << endl;
        exit(EXIT_FAILURE);
    }

    if (!opts.wHostnameFlag && !opts.wIPFlag && !opts.wIPv6Flag) {
        cerr << "Required option -w not set!" << endl;
        exit(EXIT_FAILURE);
    }
    
    //DNS

    DNSResolv DNSResolv(opts);

    DNSResolv.DNSResolvRun();

    //Get IPv4 address of queried hostname
    if (!opts.qIPFlag) {

        opts.qIP = DNSResolv.getIPAddr();
        if (!opts.qIP.empty()) opts.qIPFlag = true;
    }

    //Get hostname of queried IPv4 or IPv6 address
    if (!opts.qHostnameFlag) {

        opts.qHostname = DNSResolv.getHostname();
        if (!opts.qHostname.empty()) opts.qHostnameFlag = true;
    }

    //WHOIS

    whois(opts);

    //GEOLOCATION

    getGeolocation(opts);

    exit(EXIT_SUCCESS);
}