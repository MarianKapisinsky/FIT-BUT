/* IPK 2019 - Scanner síťových služeb
 * Marian Kapisinsky, xkapis00
 * 21.04.2019
 * */

#include <iostream>
#include <cstring>

#include <arpa/inet.h>

#include "net.hpp"

using namespace std;

uint16_t csum(const uint8_t *buf, unsigned size) {

	uint32_t sum = 0;
	unsigned i;

	/* Accumulate checksum */
	for (i = 0; i < size - 1; i += 2)
	{
		uint16_t word16 = *(uint16_t *) &buf[i];
		sum += word16;
	}

	/* Handle odd-sized case */
	if (size & 1)
	{
		uint16_t word16 = (uint8_t) buf[i];
		sum += word16;
	}

	/* Fold to get the ones-complement result */
	while (sum >> 16) sum = (sum & 0xFFFF)+(sum >> 16);

	/* Invert to get the negative in ones-complement arithmetic */
	return ~sum;
}

void set_iph(ipv4_header_t *iph, const char *src_ip, const char *dst_ip, const int protocol) {

	iph->vhl = 0x45; 
	iph->tos = 0;

	if ( protocol == IPPROTO_TCP ) {
		iph->length	= sizeof(ipv4_header_t) + sizeof(tcp_header_t);
	}
	else {
		iph->length	= sizeof(ipv4_header_t) + sizeof(udp_header_t);
	}

	iph->id	= htons(rand());
	iph->offset	= 0;
	iph->ttl = 64;
	iph->protocol = protocol;
	iph->checksum = 0;
	inet_aton(src_ip, &iph->src);
	inet_aton(dst_ip, &iph->dst);
}

void set_tcph(tcp_header_t *tcph, const char *src_ip, const char *dst_ip, const unsigned dst_port) {

	tcph->src_port = htons(43251);
	tcph->dst_port = htons(dst_port);
	tcph->seq_num = htonl(1);
	tcph->ack_num = 0;
	tcph->offset = 5;
	tcph->syn = 1; //Set SYN flag
	tcph->window = htons(1480);
	tcph->checksum = 0;
	tcph->urgent_p = 0;

	psheader_t psh;
	uint8_t *psdatagram;

	bzero(&psh, sizeof(psheader_t));
	inet_aton(src_ip, &psh.src);
	inet_aton(dst_ip, &psh.dst);
	psh.protocol = IPPROTO_TCP;
	psh.length = htons(sizeof(tcp_header_t));

	psdatagram = (uint8_t *) malloc(sizeof(psheader_t) + sizeof(tcp_header_t));
	bzero(psdatagram, (sizeof(psheader_t) + sizeof(tcp_header_t)));

	memcpy(psdatagram, &psh, sizeof(psheader_t));
	memcpy(psdatagram + sizeof(psheader_t), tcph, sizeof(tcp_header_t));

	tcph->checksum = csum(psdatagram, (sizeof(psheader_t) + sizeof(tcp_header_t))); //Calcute checksum
	free(psdatagram);
}

void set_udph(udp_header_t *udph, const char *src_ip, const char *dst_ip, const unsigned dst_port) {

	udph->src_port = htons(43252);
	udph->dst_port = htons(dst_port);
	udph->length = htons(sizeof(udp_header_t));
	udph->checksum = 0;

	psheader_t psh;
	uint8_t *psdatagram;

	bzero(&psh, sizeof(psheader_t));
	inet_aton(src_ip, &psh.src);
	inet_aton(dst_ip, &psh.dst);
	psh.protocol = IPPROTO_UDP;
	psh.length = htons(sizeof(udp_header_t));

	psdatagram = (uint8_t *) malloc(sizeof(psheader_t) + sizeof(udp_header_t));
	bzero(psdatagram, (sizeof(psheader_t) + sizeof(udp_header_t)));

	memcpy(psdatagram, &psh, sizeof(psheader_t));
	memcpy(psdatagram + sizeof(psheader_t), udph, sizeof(udp_header_t));

	udph->checksum = csum(psdatagram, (sizeof(psheader_t) + sizeof(udp_header_t))); //Calcute checksum
	free(psdatagram);
}

void set_datagram(uint8_t *datagram, const char *src_ip, const char *dst_ip, const unsigned dst_port, const int protocol) {

	ipv4_header_t *iph = (ipv4_header_t *) datagram;

	set_iph(iph, src_ip, dst_ip, protocol);
	
	if ( protocol == IPPROTO_TCP ) {
		tcp_header_t *tcph = (tcp_header_t *) (datagram + sizeof(ipv4_header_t));
		set_tcph(tcph, src_ip, dst_ip, dst_port);
		iph->checksum = csum(datagram, (sizeof(ipv4_header_t) + sizeof(tcp_header_t)));
	}
	else {
		udp_header_t *udph = (udp_header_t *) (datagram + sizeof(ipv4_header_t));
		set_udph(udph, src_ip, dst_ip, dst_port);
		iph->checksum = csum(datagram, (sizeof(ipv4_header_t) + sizeof(udp_header_t)));
	}
}