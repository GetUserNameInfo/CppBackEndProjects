#include "Acceptor.h"
#include "FtpServer.h"

Acceptor::Acceptor( int fd, std::string dir) :
fd(fd),dir(dir),closeConn(false),uploadCmd(false), downloadCmd(false) {
    this->fo = new FileOpt(this->dir);
}

Acceptor::~Acceptor() {
    std::cout << "Connection terminated closed! Fd=" << fd << std::endl;
    delete this->fo;
    close(this->fd); 
    this->dirs.clear();
    this->files.clear();
}

//读取通信套接字数据
void Acceptor::respondToQuery() {
    char buffer[BUFFER_SIZE];
    int bytes;
    bytes = recv(this->fd, buffer, sizeof(buffer), 0);
    
    if ( bytes>0 ) {
        std::string cliCmd = std::string(buffer, bytes); 
        std::cout << "parse command" << std::endl;
        std::string res = this->cmdParse(cliCmd);
        this->sendToClient(res);
    } else if( bytes == 0 ) { //客户端连接断开
        std::cout << "Conn Failed" << std::endl;
        this->uploadCmd = false;
        this->downloadCmd = false;
        this->closeConn=true;
    } else { //出错了
        
    }
}

//返回数据
void Acceptor::sendToClient(char* response, unsigned long length) {
    unsigned int bytesSend = 0;
    while ( bytesSend<length ) {
        int ret = send(this->fd, response+bytesSend, length-bytesSend, 0); 
        if (ret <= 0) return;
        bytesSend += ret;
    }
}

void Acceptor::sendToClient(std::string response) {
    unsigned int bytesSend = 0;
    while ( bytesSend < response.length() ) {
        int ret = send( this->fd, response.c_str()+bytesSend, response.length()-bytesSend, 0);
        if (ret<=0) return;
        bytesSend += ret;
    }
}

//获取Conn信息
int Acceptor::getFD() {
    return this->fd;
}

bool Acceptor::getCloseRequestStatus() {
    return this->closeConn;
}

//指令解析
std::string Acceptor::cmdParse(std::string& cmd) {
    std::string res = "";
    this->uploadCmd = false;

    std::vector<std::string> cmdAndParam=this->detachParams(cmd);

    if ( cmdAndParam.size() == 1 ) {           //只有指令
        if (this->cmdEquals(cmdAndParam[0], "list")) {
            std::string curDir = "./TestDir/./";
            this->dirs.clear(); this->files.clear();
            this->fo->browse(curDir,dirs,files);
            for (int j = 0; j < dirs.size() ; j++) res += dirs[j] + "\t";
            res+="\n";
            for (int i = 0; i < files.size(); i++) res += files[i] + "\n";
        } else if (this->cmdEquals(cmdAndParam[0], "pwd")) {
            std::cout << "Get Current Working Dir..." << std::endl;
            res = this->fo->getCurDir(false);
        } else if (this->cmdEquals(cmdAndParam[0], "getparentdir")) {
            std::cout << "Parent dir of working dir requested" << std::endl;
            res = this->fo->getParentDir();
        } else if (this->cmdEquals(cmdAndParam[0], "quit")) {
            std::cout << "Shutdown Connection" << std::endl;
            this->closeConn = true;
        } else { 
            std::cout << "Unknown command!" << std::endl;
            cmdAndParam.clear();
        }
    } else if ( cmdAndParam.size() > 1 ) {     //有参数
        std::string& parameter = cmdAndParam[1];
        if (this->cmdEquals(cmdAndParam[0], "ls")) {
            std::cout << "Browsing files of directory '" << parameter << "'" << std::endl;
            this->dirs.clear(); this->files.clear();
            this->fo->browse(parameter,dirs,files);
            for (int j = 0; j < dirs.size() ; j++) res += dirs[j] + "\t";
            res+="\n";
            for (int i = 0; i < files.size(); i++) res += files[i] + "\n";
        } else if (this->cmdEquals(cmdAndParam[0], "download")) {
            this->downloadCmd = true;
            std::cout << "Preparing download '" << parameter << "'..." << std::endl;
            unsigned long lenInBytes = 0;
            char* fileBlock;
            unsigned long readBytes = 0;
            std::stringstream st;
            if ( !this->fo->openFile(parameter) ) {
                st.clear();
                fileBlock = this->fo->readFileBlock(lenInBytes);
                st << lenInBytes;
                readBytes += lenInBytes; 
                this->sendToClient(fileBlock,lenInBytes);
            }
            this->closeConn = true;
        } else if (this->cmdEquals(cmdAndParam[0], "upload")) {
            this->uploadCmd = true;
            std::cout << "Preparing Upload '" << parameter << "'" << std::endl;
            //客户端把数据写入对应的通信Socket中，从服务器从TCP协议栈的缓冲区中取数据加载到content中。
            std::string content="Hello World";
            res = this->fo->beginWriteFile(parameter);
        } else if (this->cmdEquals(cmdAndParam[0], "cd")) {
            if ( !this->fo->changeDir(parameter) )
                std::cout << "Switch to '" << parameter << "' Successful!" << std::endl;
            else 
                std::cerr << "ERROR: Switch Failed!" << std::endl;
            res = this->fo->getCurDir(false);
        } else {
            std::cout << "Unknown command!" << std::endl;
            cmdAndParam.clear();
            cmd = "";
            res = "ERROR: Unknown command!";
        }
    } else if ( !cmdAndParam[0].empty() ) {    //空指令
        std::cout << "Unknown command!" << std::endl << std::endl; 
        cmdAndParam.clear();
    }

    res += "\n";
    return res;
}

bool Acceptor::cmdEquals(std::string& cmd, const std::string& tmpCmd) {
    std::transform(cmd.begin(), cmd.end(),cmd.begin(), tolower);
    int found = cmd.find(tmpCmd);
    return (found != std::string::npos); 
}

std::vector<std::string> Acceptor::detachParams(const std::string& cmd) {
    std::vector<std::string> res = std::vector<std::string>();
    std::size_t prevPos = 0 , pos;

    if ( (pos=cmd.find(SEPARATOR, prevPos),pos) != std::string::npos )
        res.emplace_back( cmd.substr( int(prevPos) , int(pos-prevPos) ) ); 

    if ( cmd.length() > (pos+1) ) 
        res.emplace_back( cmd.substr(int(pos+1)) ); 
    
    return res;
}