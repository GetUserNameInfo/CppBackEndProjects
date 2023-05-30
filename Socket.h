#ifndef _SOCKET_H
#define	_SOCKET_H

#include "Acceptor.h"
#include "Utility.h"

#include <cstdio>
#include <cstdlib>

#include <string>

//事件模式基类
class EventMode {
public:
    virtual void init()=0;
    virtual int start()=0;
    virtual ~EventMode(){};
private:
    virtual int NewConnectionHandler()=0;
};

//Select模式
class Select_Mode : public EventMode{
public:
    void init() override;
    int start() override;
    Select_Mode(int port, std::string dir);
    ~Select_Mode() override;
    
private:
    int NewConnectionHandler() override;

    int serverSocket;                        //服务器Socket
    struct sockaddr_in addr;                 //Addr信息
    socklen_t addrLen;                       //Addr长度
    int port;                                //端口
    int maxConInQuery;                       //listen最大的连接数
    int highSock;                            //最大fd
    std::vector<Acceptor*> connPool;         //连接对象数组
    std::string dir;                         //根目录
    fd_set allfds;                           //select监听的全部fd
};

//Event Separation Mode
class EventSeparationMode{
public:
    EventSeparationMode(EventMode* mode):curMode(mode){};
    void runMode();
private:
    EventMode* curMode;
};

#endif	/* _SOCKET_H */

