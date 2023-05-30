#ifndef _ACCEPTOR_H
#define	_ACCEPTOR_H

#include "FileOpt.h"

#include <iostream>
#include <vector>
#include <unordered_map>
#include <string>
#include <algorithm> 

#include <cstdlib>
#include <sys/fcntl.h>
#include <sys/unistd.h>
#include <sys/types.h>

#define SEPARATOR " "

class Acceptor {
public:
    Acceptor( int fd, std::string dir );
    virtual ~Acceptor();

    std::string cmdParse(std::string& cmd);
    std::vector<std::string> detachParams(const std::string& cmd);

    void respondToQuery();

    int getFD();
    bool getCloseRequestStatus();

private:
    bool cmdEquals(std::string& cmd, const std::string& tmpCmd);

    void sendToClient(char* response, unsigned long length);
    void sendToClient(std::string response);

    FileOpt* fo;                   //fileopt对象
    int fd;                        //通信套接字

    std::vector<std::string> dirs; //目录集合
    std::vector<std::string> files;//文件集合

    std::string dir;               //起始目录

    bool uploadCmd;                //上传标记
    bool downloadCmd;              //下载标记
    bool closeConn;                //关闭当前连接标志
};

#endif	/* _ACCEPTOR_H */
