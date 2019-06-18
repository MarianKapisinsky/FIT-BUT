/* IPK 2019 - Scanner síťových služeb
 * Marian Kapisinsky, xkapis00
 * 21.04.2019
 * */
 
#include <iostream>

#include <cstring>
#include <csignal>
#include <unistd.h>
#include <errno.h>

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <net/if.h>

#include "ipk-scan.hpp"

using namespace std;

void getLocalIPAddress(args_t *args) {
	
	int sockfd;

    if( (sockfd = socket(PF_INET, SOCK_DGRAM, 0)) < 0 ) {

        perror("E: socket()");
        exit(EXIT_FAILURE);
    }

	struct if_nameindex* pIndex = 0;
  	struct if_nameindex* pIndex2 = 0;

	pIndex = pIndex2 = if_nameindex(); //Get network device names and indexes

	while ((pIndex != NULL) && (pIndex->if_name != NULL)) {

		struct ifreq ifr;

		strncpy(ifr.ifr_name, pIndex->if_name, IFNAMSIZ);

		if (ioctl(sockfd, SIOCGIFFLAGS, &ifr) < 0) { //Get device flags

			perror("E: ioctl(SIOCGIFFLAGS)");
			close(sockfd);
			exit(EXIT_FAILURE);
		}

		if (ifr.ifr_flags & IFF_LOOPBACK) { //If device is a loopback, continue search for non-loopback 
			
			++pIndex;
			continue;
		}

		if (ioctl(sockfd, SIOCGIFADDR, &ifr) < 0) { //Get address of device

			if (errno == EADDRNOTAVAIL) {

				++pIndex;
				continue;
			}

			perror("E: ioctl(SIOCGIFADDR)");
			close(sockfd);
			exit(EXIT_FAILURE);
		}

		strcpy(args->ifr, ifr.ifr_name); //Copy device name
		strcpy(args->src_ip, inet_ntoa(((struct sockaddr_in*) &ifr.ifr_addr)->sin_addr)); //Copy device address

		if_freenameindex(pIndex2);
		close(sockfd);
		break;
	}	
}

void getInterfaceIPAddress(args_t *args, const char *ifrname) {
	
	int sockfd;

    if( (sockfd = socket(PF_INET, SOCK_DGRAM, 0)) < 0 ) {

        perror("E: socket()");
        exit(EXIT_FAILURE);
    }

	struct ifreq ifr;

	strncpy(ifr.ifr_name, ifrname, IFNAMSIZ);

	if (ioctl(sockfd, SIOCGIFFLAGS, &ifr) < 0) { //Get device flags

		perror("E: ioctl(SIOCGIFFLAGS)");
		close(sockfd);
		exit(EXIT_FAILURE);
	}

	if (ifr.ifr_flags & IFF_LOOPBACK) { //If device is a loopback, abort program
		
		fprintf(stderr, "Please choose non-loopback device\n");
		close(sockfd);
		exit(EXIT_FAILURE);
	}

	if (ioctl(sockfd, SIOCGIFADDR, &ifr) < 0) { //Get address of given device

		perror("E: ioctl(SIOCGIFADDR)");
		close(sockfd);
		exit(EXIT_FAILURE);
	}

	strcpy(args->src_ip, inet_ntoa(((struct sockaddr_in*) &ifr.ifr_addr)->sin_addr)); //Copy device address
	close(sockfd);
}