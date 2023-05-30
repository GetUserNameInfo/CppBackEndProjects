#include "Utility.h"

int U_Socket(int domain,int type,int protocol) {
    int fd=socket(domain,type,protocol);
    if( fd == -1 ) {
        std::cerr << "socket() failed" << std::endl;
        return U_ERROR;
    }
    return fd;
}

int U_Bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
    int ret=bind(sockfd,addr,addrlen);
    if( ret == -1 ) {
        std::cerr << ("bind() failed") << std::endl;
        return U_ERROR;
    }
    return U_SUCCESS;
}

int U_Listen(int sockfd, int backflag) {
    int ret=listen(sockfd, backflag);
    if ( ret == -1 ) {
        std::cerr << ("listen () failed") << std::endl;
        return U_ERROR;
    }
    return U_SUCCESS;
}

int U_Accept(int sockfd, struct sockaddr* cliAddr, socklen_t* socklen) {
    int connfd=accept(sockfd,cliAddr,socklen);
    if ( connfd == -1 ) {
        std::cerr << "Error while accepting client" << std::endl;
        return U_ERROR; 
    }
    return connfd;
}

int U_Connect() {
    return U_SUCCESS;
}

void U_Close(int sockfd) {
    close(sockfd);
}

int U_SetNonBlocking(int sockfd) {
    int sflags = fcntl(sockfd, F_GETFL); 
    sflags = (sflags | O_NONBLOCK);
    if ( fcntl(sockfd,F_SETFL,sflags) < 0 ) {
        std::cerr << "Error setting socket to non-blocking" << std::endl;
        return U_ERROR;
    }
    return U_SUCCESS;
}

int U_SetPortReuse(int sockfd, int level, int optname, const void *optval, socklen_t optlen) {
    if( setsockopt(sockfd, level, optname, optval, optlen) < 0 ) {
        std::cerr << "setsockopt() failed" << std::endl;
        return U_ERROR;
    }
    return U_SUCCESS;
}