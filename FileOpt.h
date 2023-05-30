#ifndef _FILEOPT_H
#define	_FILEOPT_H

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <list>

#include <stdint.h>
#include <unistd.h>
#include <cstdlib>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <locale.h>
#include <pwd.h>
#include <grp.h>


#define BUFFER_SIZE              4096
#define ROOT_DIR_MASK            "<root>"
#define SLASH                    "/"

class FileOpt {
public:
    FileOpt(std::string dir);
    virtual ~FileOpt();

    void browse(const std::string& dir, std::vector<std::string> &dirs, std::vector<std::string> &files);
    std::string getCurDir(bool showRootPath = true);    
    std::string getParentDir();  
    bool changeDir(const std::string& newPath);    

    int openFile(std::string fileName);
    char* readFileBlock(unsigned long &sizeInBytes);

    int beginWriteFile(std::string fileName);
    int writeFileAtOnce(std::string fileName, char* content);
    int writeFileBlock(std::string content);
    void closeWriteFile();

    bool dirCanBeOpenend(const std::string& dir);
    bool dirIsBelowServerRoot(std::string dirName);
    
private:
    void getValidFile(std::string &fileName);
    void stripRoot(std::string &dirOrFile);
    static void IntToString(int i, std::string &res);
    std::vector<std::string> detachPath(const std::string& path);

    std::ofstream curWriteFile;          //用于读的流
    std::ifstream curReadFile;           //用于写的流
    std::list<std::string> completePath; //全路径的集合
 
    const std::string rootDir;           //记录根目录(只读)
    int rootDirCnt;                      //记录根目录的迭代器
};

#endif	/* _FILEOPT_H */
