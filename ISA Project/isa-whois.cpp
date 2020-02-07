/* ISA 2019 - Whois tazatel
 * Marian Kapisinsky, xkapis00
 * 18.11.2019
 * */

#include <iostream>
#include <string>
#include <regex>
#include <vector>

#include "isa-whois.hpp"

using namespace std;

string sendQuery(opts_t opts, struct sockaddr *addr, string query) {

    int sockfd;
    int recvSize;

    char buffer[65535];
    string response;

    memset(&buffer, 0, sizeof(buffer));

    if (opts.wIPv6Flag) {
        sockfd = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
        if (connect(sockfd, addr, sizeof(struct sockaddr_in6)) != 0) { 
            cerr << "Connection with the server failed!" << endl; 
            exit(EXIT_FAILURE);
        }
    }
    else {
        sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (connect(sockfd, addr, sizeof(struct sockaddr_in)) != 0) { 
            cerr << "Connection with the server failed!" << endl;
            exit(EXIT_FAILURE);
        }
    }

    //Send the query
    if (send(sockfd, query.c_str(), query.length() , 0) < 0) {
		cerr << "Send failed!" << endl;
        exit(EXIT_FAILURE);
	}

    //Read the response
    while ((recvSize = recv(sockfd, buffer, sizeof(buffer), 0)) > 0 ) {
        response += buffer;
    }

    return response;
}

void parseJson(string json) {

    string s = json;
    string line;
    string delimiter;
    size_t pos = 0;
    string token;
    vector<string> v;  

    delimiter = ",";

    s.erase(0, 1);
    
    while ((pos = s.find(delimiter)) != string::npos) {

        token = s.substr(0, pos);

        if (token.find("location") != string::npos)
            break;

        v.push_back(token);
        s.erase(0, pos + delimiter.length());
    }

    for (size_t i = 2; i < v.size(); ++i) {
        
        line = v.at(i);

        if (line.find("_code") != string::npos)
            continue;

        line.erase(remove(line.begin(), line.end(), '\"' ), line.end());

        if ( (pos = line.find("_name")) != string::npos) {
            line.erase(pos, 5);
        }
        
        if ( (pos = line.find(":")) != string::npos) {

            if (line.find("city") != string::npos || line.find("zip") != string::npos || line.find("region") != string::npos)
                line.insert(pos + 1, "\t\t");
            else
                line.insert(pos + 1, "\t");
        }
        
        cout << line << endl;
    }
}

void whois(opts_t opts) {

    cout << "=== WHOIS ===" << endl;

    struct sockaddr_in whoisServerAddr;
    struct sockaddr_in6 whoisServerAddrv6;

    string query;
    string response;
    size_t noEntries;
    size_t invalidRequest;

    if (opts.wHostnameFlag) {
        hostent *host = gethostbyname(opts.wHostname.c_str()); //Get host IPv4 address
        if(!host) {

            cerr << opts.wHostname << " is unavailable!" << endl;
            return;
        }

        in_addr *addr = (in_addr *)host->h_addr;
        whoisServerAddr.sin_addr.s_addr = inet_addr(inet_ntoa(*addr)); //Set the address
    }
    else if (opts.wIPFlag) {
        whoisServerAddr.sin_addr.s_addr = inet_addr(opts.wIP.c_str()); //Set IPv4 address
    }
    else {
        inet_pton(AF_INET6, opts.wIPv6.c_str(), &whoisServerAddrv6.sin6_addr); //Set IPv6 address
        whoisServerAddrv6.sin6_family = AF_INET6;
        whoisServerAddrv6.sin6_port = htons(WHOIS_PORT);
    }

    if (opts.wHostnameFlag || opts.wIPFlag) {
        whoisServerAddr.sin_family = AF_INET;
        whoisServerAddr.sin_port = htons(WHOIS_PORT);    
    }

    //Create the query
    if (opts.wHostname.compare("whois.nic.cz") == 0 || opts.wIP.compare("217.31.205.42") == 0 || opts.wIP.compare("2001:1488::2:0:0:0:2") == 0) { 
        query += "-rT domain ";
        query += opts.qHostname;
    }
    else if (opts.wHostname.compare("whois.ripe.net") == 0 || opts.wIP.compare("193.0.6.135") == 0 || opts.wIP.compare("2001:67c:2e8:22::c100:687") == 0) { 
        query += "-T inetnum,inet6num -r ";
        if (opts.qIPv6Flag)
            query += opts.qIPv6;
        else
            query += opts.qIP;
    }
    else {
        if (opts.qIPv6Flag)
            query += opts.qIPv6;
        else
            query += opts.qIP;
    }

    if (query.empty()) {
        cout << "Nothing to search for here!" << endl;
        return;
    }

    query += "\r\n";
    
    //Create socket and connect the the WHOIS server
    if (opts.wIPv6Flag)
        response = sendQuery(opts, (struct sockaddr *)&whoisServerAddrv6, query);
    else 
        response = sendQuery(opts, (struct sockaddr *)&whoisServerAddr, query);

    noEntries = response.find("ERROR:101");
    invalidRequest = response.find("ERROR:108");

    if (invalidRequest != string::npos) {

        cerr << "Invalid request" << endl; 
    }
    else if (noEntries != string::npos) {

        cout << "No entries found for " << opts.qHostname << " on " << opts.wHostname << endl;       
    }
    else {
        //Parse response - remove comments and print data
        istringstream responseStream;
        string line;
        string registrant;
        string org;
        string admin;
        bool orgFlag = false;
        bool adminFlag = false;

        responseStream.str(response);

        while (getline(responseStream, line)) {

            if (line[0] == '%' || line[0] == '#' || line[0] == '\r' || line.empty()) continue;

            if (opts.wHostname.compare("whois.ripe.net") == 0 || opts.wIP.compare("193.0.6.135") == 0 || opts.wIP.compare("2001:67c:2e8:22::c100:687") == 0) {

                if (line.find("inetnum:") != string::npos) cout << line << endl;
                if (line.find("inet6num:") != string::npos) cout << line << endl;
                if (line.find("netname:") != string::npos) cout << line << endl;
                if (line.find("descr:") != string::npos) cout << line << endl;
                if (line.find("country:") != string::npos) cout << line << endl;

                if (line.find("admin-c:") != string::npos) {
                    if (!adminFlag) {
                        admin = line;
                        adminFlag = true;
                    }
                    cout << line << endl;
                }

                if (line.find("org:") != string::npos) {
                    if (!orgFlag) {
                        org = line;
                        orgFlag = true;
                    }
                    cout << line << endl;
                }
            }
            else if (opts.wHostname.compare("whois.nic.cz") == 0 || opts.wIP.compare("217.31.205.42") == 0 || opts.wIP.compare("2001:1488::2:0:0:0:2") == 0) {

                if (line.find("registrant:") != string::npos) registrant = line;
                if (line.find("changed:") != string::npos) continue;
                if (line.find("expire:") != string::npos) continue;
                cout << line << endl;
            }
            else {
                cout << line << endl;
            }   
        }

        if (opts.wHostname.compare("whois.ripe.net") == 0 || opts.wIP.compare("193.0.6.135") == 0 || opts.wIP.compare("2001:67c:2e8:22::c100:687") == 0) {
            
            line.clear();
            response.clear();
            responseStream.clear();
            query.clear();

            if (org.empty() && admin.empty()) {
                cerr << "Nothing to search for here!" << endl;
                return;
            }

            if (orgFlag) {
                query += "-T organisation -r ";
                query += org.erase(0, 13);
                query += "\r\n";
            }
            else {
                query += "-T person,role -r ";
                query += admin.erase(0, 16);
                query += "\r\n";
            }

            if (opts.wIPv6Flag)
                response = sendQuery(opts, (struct sockaddr *)&whoisServerAddrv6, query);
            else 
                response = sendQuery(opts, (struct sockaddr *)&whoisServerAddr, query);

            responseStream.str(response);

            while (getline(responseStream, line)) {
                if (line[0] == '%' || line[0] == '#' || line.empty()) continue;
                if (line.find("address:") != string::npos) cout << line << endl;
                if (line.find("phone:") != string::npos) cout << line << endl;
            }
        }
    
        if (opts.wHostname.compare("whois.nic.cz") == 0 || opts.wIP.compare("217.31.205.42") == 0 || opts.wIP.compare("2001:1488::2:0:0:0:2") == 0) {

            line.clear();
            response.clear();
            responseStream.clear();
            query.clear();

            if (registrant.empty()) {
                cerr << "Nothing to search for here!" << endl;
                return;
            }

            query += "-rT contact ";
            query += registrant.erase(0, 14);
            query += "\n";

            if (opts.wIPv6Flag)
                response = sendQuery(opts, (struct sockaddr *)&whoisServerAddrv6, query);
            else 
                response = sendQuery(opts, (struct sockaddr *)&whoisServerAddr, query);

            responseStream.str(response);

            while (getline(responseStream, line)) {
                if (line[0] == '%' || line[0] == '#' || line.empty()) continue;
                if (line.find("address:") != string::npos) cout << line << endl;
                if (line.find("name:") != string::npos) cout << line << endl;
            }
        }
    }
}

void getGeolocation(opts_t opts) {

    cout << "=== GEOLOCATION ===" << endl;

    int sockfd;
    int recvSize;
    struct sockaddr_in geoLocServerAddr;

    geoLocServerAddr.sin_addr.s_addr = inet_addr("23.246.243.51");
    geoLocServerAddr.sin_family = AF_INET;
    geoLocServerAddr.sin_port = htons(80);

    char buffer[65535];
    string query;
    string response;

    memset(&buffer, 0, sizeof(buffer));

    if (opts.qIPv6Flag)
        query += "GET /" + opts.qIPv6 + "?access_key=025ecb178cc44c63b29e06db8c79c112";
    else
        query += "GET /" + opts.qIP + "?access_key=025ecb178cc44c63b29e06db8c79c112";

    query += " HTTP/1.1\r\nHost: api.ipstack.com\r\n\r\n";

    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (connect(sockfd, (struct sockaddr *)&geoLocServerAddr, sizeof(struct sockaddr_in)) != 0) { 
        cerr << "Connection with the server failed!" << endl;
        exit(EXIT_FAILURE);
    }
        
    if (send(sockfd, query.c_str(), query.length() , 0) < 0) {
        cerr << "Send failed!" << endl;
        exit(EXIT_FAILURE);
    }

    if ((recvSize = recv(sockfd, buffer, sizeof(buffer) , 0)) <= 0 ) {
        cerr << "No response received!" << endl;
    }
    response = buffer;
        
    string line;
    istringstream responseStream(response);

    getline(responseStream, line);

    if (line.find("HTTP/1.1 200 OK") == string::npos) {
        cerr << "No or bad response received!" << endl;
        return;
    }
        
    while (getline(responseStream, line)) {

        if (line[0] == '{')
            parseJson(line);
        else
            continue;
    }
}