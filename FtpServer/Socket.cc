#include "Socket.h"

//EventSeparation 
void EventSeparationMode::runMode() {
    this->curMode->init();
    this->curMode->start();
}

//Select Mode
Select_Mode::Select_Mode(int port, std::string dir):port(port),dir(dir) {
    // if ( chdir(dir.c_str()) )  
    //     std::cerr << "Directory could not changed to '" << dir << "'!" << std::endl;
}

void Select_Mode::init() {
    int reuseAllowed=1;
    this->maxConInQuery=50;

    //初始化服务器Socket
    if( (this->serverSocket=U_Socket(AF_INET,SOCK_STREAM,0),this->serverSocket)<0 ) return;
    if( U_SetPortReuse(this->serverSocket, SOL_SOCKET, SO_REUSEADDR, &reuseAllowed, sizeof(reuseAllowed))<0 ) {
        close(this->serverSocket);
        return;
    }

    this->addr.sin_family=PF_INET;
    this->addr.sin_port=htons(this->port);
    this->addr.sin_addr.s_addr=INADDR_ANY;

    if( U_SetNonBlocking(this->serverSocket)<0 ) return;
    if( U_Bind(this->serverSocket,(struct sockaddr*)&(this->addr),sizeof(this->addr))<0 ) {
        close(this->serverSocket);
        return;
    }
    if( U_Listen(serverSocket,this->maxConInQuery)<0 ) {
        close(this->serverSocket);
        return;
    }

    this->highSock = this->serverSocket; 
    std::cout << "Server started and listening at port='" << this->port 
    << "', root directory='" << this->dir << "'" << std::endl;
}

int Select_Mode::start() {
    struct timeval select_timeout;
    select_timeout.tv_sec=1;
    select_timeout.tv_usec=0;

    fd_set fds;
    FD_ZERO(&fds);FD_ZERO(&(this->allfds));
    FD_SET(this->serverSocket,&(this->allfds));

    int ret;
    while( 1 ) {
        //清除无效的连接
        for(auto iter=this->connPool.begin(); iter != this->connPool.end(); iter++) {
            if( (*iter)->getCloseRequestStatus() == true ) {
                delete (*iter); 
                this->connPool.erase(iter);
                FD_CLR((*iter)->getFD(),&(this->allfds));
                if( this->connPool.empty() || (iter == this->connPool.end()) ) break;
            }
        }

        //返回全部有反应的fds
        fds=this->allfds;
        ret=select(this->highSock+1,&fds,(fd_set*)0,(fd_set*)0,&select_timeout);
        if( ret < 0 ) {
            std::cerr << "Error calling select" << std::endl;
            return U_ERROR;
        }

        //处理有反应的fd
        if( FD_ISSET(this->serverSocket,&fds) ) this->NewConnectionHandler();
        for(int idx=0; idx < this->connPool.size(); idx++) {
            if( FD_ISSET(this->connPool[idx]->getFD(),&fds) ) {
                this->connPool[idx]->respondToQuery();
            }
        }
    }
    
    return U_SUCCESS;
}

int Select_Mode::NewConnectionHandler() {
    struct sockaddr_in cliAddr;
    socklen_t cliSockLen=sizeof(cliAddr);

    int connFd;
    if((connFd=U_Accept(this->serverSocket,(struct sockaddr*)&cliAddr,&cliSockLen),connFd)<0) return U_ERROR;
    
    if( U_SetNonBlocking(connFd)<0 ) {
        U_Close(connFd);
        return U_ERROR;
    };

    // char ipstr[INET6_ADDRSTRLEN];
    // int port;
    // this->addrLength = sizeof(this->addrStorage);
    // getpeername(fd, (struct sockaddr*) &this->addrStorage, &(this->addrLength));
    // std::string hostId = "";
    // if (this->addr.sin_family == AF_INET) {
    //     struct sockaddr_in* fd = (struct sockaddr_in*) &(this->addrStorage);
    //     port = ntohs(fd->sin_port);
    //     inet_ntop(AF_INET, &fd->sin_addr, ipstr, sizeof ipstr);
    //     hostId = (std::string)ipstr;
    // }

    FD_SET(connFd,&(this->allfds));
    if( connFd > this->highSock ) this->highSock=connFd;

    printf("Connection accepted: FD=%d - Slot=%lu \n", connFd, (this->connPool.size()+1));
    Acceptor* conn = new Acceptor(connFd, this->dir);
    this->connPool.emplace_back(conn);
    return U_SUCCESS;
}

Select_Mode::~Select_Mode() {
    std::cout << "Server shutdown!" << std::endl;
    U_Close(this->serverSocket);
    
    for(auto iter=this->connPool.begin();iter!=this->connPool.end();iter++) delete(*iter);
    this->connPool.clear();
}