/* IPK 2019 - Scanner síťových služeb
 * Marian Kapisinsky, xkapis00
 * 21.04.2019
 * */

#ifndef INTERFACE_H
#define INTERFACE_H

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

//Returns address of first non-loopback device, inspired by https://ubuntuforums.org/showthread.php?t=1396491
void getLocalIPAddress(args_t *args);


//Returns address of given device, if loopback abort
void getInterfaceIPAddress(args_t *args, const char *ifrname);

#endif //INTERFACE_H