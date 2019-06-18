/* IPK 2019 - Scanner síťových služeb
 * Marian Kapisinsky, xkapis00
 * 21.04.2019
 * */

#ifndef NET_H
#define NET_H

#include <netinet/ip.h>
#include <stdint.h>

#include "ipk-scan.hpp"

/* Ethernet header */

#define ETH_HEADER_LEN 16

typedef struct eth_header {
  
  //Destination address
  uint8_t dst;

  //Source address
  uint8_t src;

  //Type of next level protocol
  uint16_t type;

} eth_header_t;

/* IPv4 header */

#define IP_HEADER_LEN 20

typedef struct ipv4_header {

  //Version and header length
  uint8_t vhl;

  //Type of Service
  uint8_t tos;

  //Total length
  uint16_t length;

  //Identification
  uint16_t id;

  //Flags and Fragment offset
  uint16_t offset;

  //Time to Live
  uint8_t ttl;

  //Next level protocol
  uint8_t protocol;

  //Header checksum
  uint16_t checksum;

  //Source address
  struct in_addr src;

  //Destination address
  struct in_addr dst;
  
} ipv4_header_t;

/* IPv6 Header */

#define IPV6_HEADER_LEN 40

typedef struct ipv6_header {

  //Version
  uint8_t version:4;

  //Traffic class
  uint8_t traffic_class;

  //Flow label
  uint32_t flow_label:20;

  //Payload length
  uint16_t length;

  //Next header
  uint8_t next_header;

  //Hop Limit
  uint8_t hop_limit;

  //Source address
  struct in6_addr src;

  //Destination address
  struct in6_addr dst;

} ipv6_header_t;

/* Pseudo header */

#define PSHEADER_LEN 12

typedef struct psheader {

  //Source address
  struct in_addr src;

  //Destination address
  struct in_addr dst;

  //Zeros
  uint8_t zeros;

  //Protocol
  uint8_t protocol;

  //Length
  uint16_t length;

} psheader_t;

/* TCP header */

#define TCP_HEADER_LEN 20
#define TCP_OPTIONS_LEN 20

typedef struct tcp_header {

  //Source port
  uint16_t src_port;

  //Destination port
  uint16_t dst_port;

  //Sequence number
  uint32_t seq_num;

  //Acknowledgment number
  uint32_t ack_num;

  //Data offset, reserved and flags
  uint8_t reserved:4, offset:4, fin:1, syn:1, rst:1, psh:1, ack:1, urg:1;  // 4 + 6 + 6 bits

  //Window
  uint16_t window;

  //Checksum
  uint16_t checksum;

  //Urgent pointer
  uint16_t urgent_p;
  
  //Options
  //uint8_t options[TCP_OPTIONS_LEN];

} tcp_header_t;

/* UDP header */

#define UDP_HEADER_LEN 8

typedef struct udp_header {
	
  //Source port
  uint16_t src_port;
	
  //Destination port
  uint16_t dst_port;
	
	//Total length
  uint16_t length;
	
  //Checksum
  uint16_t checksum;
	
} udp_header_t;

/* ICMP Destination Unreachable Message */

typedef struct icmp_dum {

  //Type
  uint8_t type;

  //Code
  uint8_t code;

  //Checksum
  uint16_t checksum;

  //Unused
  uint32_t unused;

  //IP header
  ipv4_header_t iph;

} icmp_dum_t;

/* Functions */

//Calculates checksum for IP and TCP, Source: https://github.com/praveenkmurthy/Raw-Sockets/blob/master/tcp_handler.c
uint16_t csum(const uint8_t *buf, unsigned size);

//Fills IPv4 header
void set_iph(ipv4_header_t *iph, const char *src_ip, const char *dst_ip, const int protocol);

//Fills TCP header
void set_tcph(tcp_header_t *tcph, const char *src_ip, const char *dst_ip, const unsigned dst_port);

//Fills UDP header
void set_udph(udp_header_t *udph, const char *src_ip, const char *dst_ip, const unsigned dst_port);

//Prepares datagrams and fills IP checksum
void set_datagram(uint8_t *datagram, const char *src_ip, const char *dst_ip, const unsigned dst_port, const int protocol);

#endif //NET_H