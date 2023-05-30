#include "FtpServer.h"

int main(int argc, char** argv) {
    unsigned int port = 4242;
    std::string dir = "./";

    if( argc > 1 ) port = atoi(argv[1]);
    if( argc > 2 ) {
        FileOpt* fo = new FileOpt(dir);
        if ( fo->dirCanBeOpenend(argv[2]) ) dir = argv[2];
        else std::cout << "Invalid DirPath ('" << argv[2] << "') " << std::endl;
        delete fo;
    } 
    std::cout << "Usage: FTP Server\r\nUsing Port={" << port << "} , Default DirPath={" << dir << "}" << std::endl;

    EventMode* curMode=new Select_Mode(port,dir);
    EventSeparationMode* server=new EventSeparationMode(curMode);
    server->runMode();
    delete curMode;
    delete server;

    return U_SUCCESS;
}
