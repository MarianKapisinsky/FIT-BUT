/* IPK 2019 - Scanner síťových služeb
 * Marian Kapisinsky, xkapis00
 * 21.04.2019
 * */
 
#include <iostream>
#include <vector>
#include <regex>

#include <cstring>
#include <csignal>
#include <unistd.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <pcap.h> //https://www.tcpdump.org/pcap.html

#include "interface.hpp"
#include "net.hpp"
#include "ipk-scan.hpp"

using namespace std;

//IPv4, Host and Optarg string validation regular expressions
regex ipv4_r("^(([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5]).){3}([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])$");
regex host_r("^(([a-zA-Z0-9]|[a-zA-Z0-9][a-zA-Z0-9-]*[a-zA-Z0-9]).)*([A-Za-z0-9]|[A-Za-z0-9][A-Za-z0-9-]*[A-Za-z0-9])$");
regex optarg_r("^(([0-9]+(,|-))*)([0-9]*)$");

pcap_t *handle;	//pcap session handle

/************************************************************/

void check_optarg(vector<string> *ports, string optarg) {

	int count = 0;
	for (size_t i = 0; i < optarg.length(); i++) 
		if (optarg.at(i) == '-') count++;

	if (count == 1) {

		size_t dpos = optarg.find("-");

		int from = stoi(optarg.substr(0, dpos));
		int to = stoi(optarg.substr(dpos+1));

		if ( (from > to) || (from < 1) || (from > 65535) || (to < 1) || (to > 65535) ) {

			fprintf(stderr, "Invalid port range: %s\n", optarg.c_str());
			exit(EXIT_FAILURE);
		}

		for (int i = from; i <= to; i++)
				(*ports).push_back(to_string(i));
	}
	else if ( count > 1 ) {

		fprintf(stderr, "Invalid port range: %s\n", optarg.c_str());
		exit(EXIT_FAILURE);
	}
	else {
		
		if ( (stoi(optarg) < 1) || (stoi(optarg) > 65535) ) {

			fprintf(stderr, "Invalid port number: %s\n", optarg.c_str());
			exit(EXIT_FAILURE);
		}

		(*ports).push_back(optarg);
	}		
}

/************************************************************/

args_t parse_args(int argc, char **argv) {

	if ( argc == 1 ) {

		fprintf(stderr, "No arguments set\n");
		exit(EXIT_FAILURE);
	}

	bool pt = false, pu = false, ifr = false;

	args_t args;

	bzero(args.src_ip, sizeof(args.src_ip));
	bzero(args.dst_ip, sizeof(args.dst_ip));
	bzero(args.dst_hostname, sizeof(args.dst_hostname));

	string optarg;
	string ports;
	string delimiter = ",";
	size_t pos = 0;
	
	for (int i = 1; i < (argc - 1) ; ++i) {

		pos = 0;

		if (strcmp(argv[i], "-pt") == 0) { //TCP ports

			optarg = argv[i+1];

			if (!regex_match(optarg, optarg_r)) {
		
				fprintf(stderr, "Invalid optarg: %s\n", argv[i+1]);
				exit(EXIT_FAILURE);
			}
					
			while ((pos = optarg.find(delimiter)) != string::npos) {
				
				ports = optarg.substr(0, pos);

				check_optarg(&args.pt, ports);

				optarg.erase(0, pos + delimiter.length());
			}
			
			ports = optarg.substr(0, pos);

			check_optarg(&args.pt, ports);
			
			pt = true;

			i++;
		}
		else if (strcmp(argv[i], "-pu") == 0) { //UDP ports
			
			optarg = argv[i+1];

			if (!regex_match(optarg, optarg_r)) {
		
				fprintf(stderr, "Invalid optarg: %s\n", argv[i+1]);
				exit(EXIT_FAILURE);
			}

			while ((pos = optarg.find(delimiter)) != string::npos) {
				
				ports = optarg.substr(0, pos);

				check_optarg(&args.pu, ports);

				optarg.erase(0, pos + delimiter.length());
			}
			
			ports = optarg.substr(0, pos);

			check_optarg(&args.pu, ports);
			
			pu = true;

			i++;
		}
		else if (strcmp(argv[i], "-i") == 0) {

			strcpy(args.ifr, argv[i+1]);

			ifr = true;

			i++;
		}
		else {

			fprintf(stderr, "Unknown option: %s\n", argv[i]);
			exit(EXIT_FAILURE);
		}
	}

	if (!pt && !pu) { //If no arguments set, abort program

		fprintf(stderr, "No arguments set\n");
		exit(EXIT_FAILURE);
	}

	if (regex_match(argv[argc-1], ipv4_r)) {
		
		strcpy(args.dst_ip, argv[argc-1]);
	}
	else if (regex_match(argv[argc-1], host_r)) {

		hostent *host = gethostbyname(argv[argc-1]); //Get host address
		if(!host)
		{
			fprintf(stderr, "%s is unavailable\n", argv[argc-1]);
			exit(EXIT_FAILURE);
		}

		in_addr *addr = (in_addr *)host->h_addr;
		strcpy(args.dst_ip, inet_ntoa(*addr));

		strcpy(args.dst_hostname, argv[argc-1]);
	}
	else {

		fprintf(stderr, "Missing or invalid address\n");
		exit(EXIT_FAILURE);
	}

	if (!strcmp(args.dst_ip, "127.0.0.1")) {

		strcpy(args.src_ip, "127.0.0.1");
	}
	else if (ifr) {

		getInterfaceIPAddress(&args, args.ifr); //Get IP address of given device
	}
	else {

		getLocalIPAddress(&args); //Get IP address of first non-loopback device
	}

	return args;
}

/************************************************************/

void timeout(int signum) {

	pcap_breakloop(handle);
}

/************************************************************/

int main(int argc, char **argv) {
	
	args_t args = parse_args(argc, argv); //Get cmd line args

	int sockfd;
	struct sockaddr_in dst_addr;

	dst_addr.sin_family = AF_INET;
	dst_addr.sin_addr.s_addr = inet_addr(args.dst_ip); //Setup destination address

	uint8_t tcp_datagram[TCP_DATAGRAM_SIZE]; //Prepare datagrams
	uint8_t udp_datagram[UDP_DATAGRAM_SIZE];

	bzero(tcp_datagram, sizeof(tcp_datagram));
	bzero(udp_datagram, sizeof(udp_datagram));

	if ( (sockfd = socket(PF_INET, SOCK_RAW, IPPROTO_RAW)) < 0 ) { //Open raw socket

		perror("E: socket()");
		exit(EXIT_FAILURE);
	}

    int flag = 1;
	if ( setsockopt(sockfd, IPPROTO_IP, IP_HDRINCL, &flag, sizeof(flag)) < 0 ) { //Tell kernel that header are included in datagrams

		perror("E: setsockopt()");
		close(sockfd);
		exit(EXIT_FAILURE);
	}

	char errbuf[PCAP_ERRBUF_SIZE];	//Error string

	struct bpf_program fp;		//The compiled filter
	char filter_exp[32];	// Thefilter expression
	bzero(filter_exp, sizeof(filter_exp));

	bpf_u_int32 mask;		//Our netmask
	bpf_u_int32 net;		//Our IP

	struct pcap_pkthdr header;	//The header that pcap gives us
	const u_char *packet;		//The actual packet 

	if (pcap_lookupnet("any", &net, &mask, errbuf) == -1) { //Find the properties for the device

		net = 0;
		mask = 0;
	}
	
	handle = pcap_open_live("any", BUFSIZ, 1, 1000, errbuf); //Open the session in promiscuous mode
	if (handle == NULL) {

		fprintf(stderr, "Couldn't open device: %s\n", errbuf);
		close(sockfd);
		exit(EXIT_FAILURE);
	}

	printf("Interesting ports on %s (%s):\n", args.dst_hostname, args.dst_ip); //Print program info
	printf("PORT\tSTATE\n"); //Print program info (portnumber/protocol and its state (open/filtered/closed))

	/* TCP Port Scan */

	vector<string>::iterator port;

	for ( port = args.pt.begin(); port != args.pt.end(); ++port ) { //Iterate through all given TCP ports

		bzero(filter_exp, sizeof(filter_exp));

		strcpy(filter_exp, "tcp src port ");
		strcat(filter_exp, (*port).c_str());

		if (pcap_compile(handle, &fp, filter_exp, 0, net) == -1) { //Compile filter
			fprintf(stderr, "Couldn't parse filter %s: %s\n", filter_exp, pcap_geterr(handle));
			close(sockfd);
			exit(EXIT_FAILURE);
		}

		if (pcap_setfilter(handle, &fp) == -1) { //Set pcap filter (tcp src port <source port>)
			fprintf(stderr, "Couldn't install filter %s: %s\n", filter_exp, pcap_geterr(handle));
			close(sockfd);
			exit(EXIT_FAILURE);
		}	

		dst_addr.sin_port = htons(stoi(*port)); //Set destination port

		bzero(tcp_datagram, sizeof(tcp_datagram));

		set_datagram(tcp_datagram, args.src_ip, args.dst_ip, stoi(*port), IPPROTO_TCP); //Prepare TCP datagram

		if ( sendto(sockfd, tcp_datagram, sizeof(tcp_datagram), 0, (struct sockaddr *)&dst_addr, sizeof(dst_addr)) <= 0 ) { //Send datagram to destination
			
			perror("E: sendto()");
			close(sockfd);
			exit(EXIT_FAILURE);
		}

		signal(SIGALRM, &timeout); //Set alarm signal to break pcap_next loop when no packet is captured after 30 seconds
		alarm(30); //Set timeout to 30 seconds

		packet = pcap_next(handle, &header); //Capture destinations reply packet

		if (!packet) { //No packet captured, resend datagram to validate if port is filtered

			if ( sendto(sockfd, tcp_datagram, sizeof(tcp_datagram), 0, (struct sockaddr *)&dst_addr, sizeof(dst_addr)) <= 0 ) {
				
				perror("E: sendto()");
				close(sockfd);
				exit(EXIT_FAILURE);
			}

			signal(SIGALRM, &timeout);
			alarm(30);

			packet = pcap_next(handle, &header);

			if (!packet) { //If again no packet is received, port is filtered

				printf("%s/tcp\tfiltered\n", (*port).c_str()); //Print port info
			} else {

				tcp_header_t *tcph = (tcp_header_t *) (packet + ETH_HEADER_LEN + IP_HEADER_LEN); //Parse packet to get TCP info

				if (tcph->rst) { //If RST Flag is set then port is closed, otherwise is open
					printf("%s/tcp\tclosed\n", (*port).c_str());  //Print port info
				}
				else {
					printf("%s/tcp\topen\n", (*port).c_str()); //Print port info
				}
			}
		}
		else {
			
			tcp_header_t *tcph = (tcp_header_t *) (packet + ETH_HEADER_LEN + IP_HEADER_LEN); //Parse packet to get TCP info

			if (tcph->rst) { //If RST Flag is set then port is closed, otherwise is open
				printf("%s/tcp\tclosed\n", (*port).c_str()); //Print port info
			}
			else {
				printf("%s/tcp\topen\n", (*port).c_str()); //Print port info
			}
		}
	}

	/* UDP Port Scan */

	for ( port = args.pu.begin(); port != args.pu.end(); ++port ) { //Iterate through all given UDP ports

		bzero(filter_exp, sizeof(filter_exp));

		strcpy(filter_exp, "src net ");
		strcat(filter_exp, args.dst_ip);
		strcat(filter_exp, " and icmp");

		if (pcap_compile(handle, &fp, filter_exp, 0, net) == -1) { //Compile filter
			fprintf(stderr, "Couldn't parse filter %s: %s\n", filter_exp, pcap_geterr(handle));
			close(sockfd);
			exit(EXIT_FAILURE);
		}

		if (pcap_setfilter(handle, &fp) == -1) { //Set pcap filter (src net <source address> and icmp)
			fprintf(stderr, "Couldn't install filter %s: %s\n", filter_exp, pcap_geterr(handle));
			close(sockfd);
			exit(EXIT_FAILURE);
		}

		dst_addr.sin_port = htons(stoi(*port)); //Set destination port

		bzero(udp_datagram, sizeof(udp_datagram));

		set_datagram(udp_datagram, args.src_ip, args.dst_ip, stoi(*port), IPPROTO_UDP); //Prepare UDP datagram

		if ( sendto(sockfd, udp_datagram, sizeof(udp_datagram), 0, (struct sockaddr *)&dst_addr, sizeof(dst_addr)) <= 0 ) { //Send datagram to destination
			
			perror("E: sendto()");
			close(sockfd);
			exit(EXIT_FAILURE);
		}

		signal(SIGALRM, &timeout); //Set alarm signal to break pcap_next loop when no packet is captured after 30 seconds
		alarm(30); //Set timeout to 30 seconds

		packet = pcap_next(handle, &header); //Capture destinations reply packet

		if (!packet) { //No packet captured, port is considered to be open

			printf("%d/udp\topen\n", stoi(*port)); //Print port info
		}
		else { //If ICMP packet type 3 with code 3 is captured, port is closed

			icmp_dum_t *icmp = (icmp_dum_t *) (packet + ETH_HEADER_LEN + IP_HEADER_LEN); //Parse packet to get ICMP info

			if (icmp->type == 3) {
				printf("%d/udp\tclosed\n", stoi(*port)); //Print port info
			}
		}
	}

	pcap_close(handle); //Close pcap handle
	close(sockfd); //Close socket

	exit(EXIT_SUCCESS);
}
