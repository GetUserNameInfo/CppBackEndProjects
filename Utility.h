#ifndef _UTILITY_H
#define _UTILITY_H

#include <iostream>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/fcntl.h>
#include <sys/param.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>



#define U_SUCCESS  0
#define U_ERROR   -1

int U_Socket(int domain,int type,int protocol);
int U_Bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int U_Listen(int sockfd, int backflag);
int U_Accept(int sockfd, struct sockaddr* cliAddr, socklen_t* socklen);
int U_Connect();
void U_Close(int sockfd);
int U_SetNonBlocking(int sockfd);
int U_SetPortReuse(int sockfd, int level, int optname, const void *optval, socklen_t optlen);

#endif /*_UTILITY_H*/