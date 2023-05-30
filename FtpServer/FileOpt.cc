#include "FileOpt.h"

FileOpt::FileOpt(std::string dir):rootDir(dir) {
    this->completePath.clear();
    for( auto& path : this->detachPath(dir) ) 
        this->completePath.push_back(path);
    this->rootDirCnt=this->completePath.size();
}

FileOpt::~FileOpt() {
    this->closeWriteFile();
    this->completePath.clear();
}

//private
void FileOpt::getValidFile(std::string &fileName) {
    std::string slash = "/";
    size_t foundSlash = 0;
    while ( (foundSlash = fileName.find_first_of(slash),(foundSlash)) != std::string::npos) {
        fileName.erase(foundSlash++,1);
    }
}

void FileOpt::stripRoot(std::string &dirOrFile) {
    size_t rootStr = 0;
    if ( dirOrFile.find_first_of(this->rootDir) == rootStr ) { 
        int rootStrLen = ((std::string)this->rootDir).length();
        dirOrFile = dirOrFile.substr( rootStrLen, dirOrFile.length()-rootStrLen); 
    }
}

void FileOpt::IntToString(int i, std::string& res) {
    std::ostringstream temp;
    temp << i;
    res = temp.str();
}

std::vector<std::string> FileOpt::detachPath(const std::string& path) {
    std::vector<std::string> dirsOfPath;
    std::string slash = "/";
    size_t pos = 0;
    size_t slashPos = 0;
    while ( ( slashPos = path.find_first_of(slash,pos) , slashPos ) != std::string::npos) {
        dirsOfPath.emplace_back(path.substr(pos,slashPos-pos));
        pos=slashPos+1;
    }
    return dirsOfPath;
}

//读文件流
int FileOpt::openFile(std::string fileName) {
    stripRoot(fileName);
    this->curReadFile.open(fileName.c_str(), std::ios::in | std::ios::binary); 
    if ( this->curReadFile.fail() ) {
        std::cout << "Reading file '" << fileName << "' failed!" << std::endl; 
        return (EXIT_FAILURE);
    }

    if ( !this->curReadFile.is_open() ) {
        std::cerr << "Unable to open file '" << fileName << " '" << std::endl;
        return (EXIT_FAILURE);
    }

    return (EXIT_SUCCESS);
}

char* FileOpt::readFileBlock(unsigned long& sizeInBytes) {
    this->curReadFile.seekg(0, std::ios::end); //移动到文件尾
    std::ifstream::pos_type size = this->curReadFile.tellg(); //获取当前指针位置
    sizeInBytes = (unsigned long)size; //获取文件大小

    this->curReadFile.seekg (0, std::ios::beg); //移动到文件头
    char* memblock = new char[size];
    this->curReadFile.read(memblock,size);
    std::cout << "Reading " << size << " Bytes" << std::endl;

    this->curReadFile.close(); //关闭读取流
    return memblock;
}

//写文件流
int FileOpt::beginWriteFile(std::string fileName) {
    stripRoot(fileName);

    this->curWriteFile.open(fileName.c_str(), std::ios::out|std::ios::binary|std::ios::app);
    if(!this->curWriteFile) {
        std::cerr << "Cannot open output file '" << fileName << "'" << std::endl;
        return (EXIT_FAILURE);
    }

    std::cout << "Beginning writing to file '" << fileName << "'" << std::endl;
    return (EXIT_SUCCESS);
}

int FileOpt::writeFileBlock(std::string content) {
    if( !this->curWriteFile ) {
        std::cerr << "Cannot write to output file" << std::endl;
        return (EXIT_FAILURE);
    }
    std::cout << "Appending to file" << std::endl;
    (this->curWriteFile) << content;
    return (EXIT_SUCCESS);
}

void FileOpt::closeWriteFile() {
    if ( this->curWriteFile.is_open() ) {
        std::cout << "Closing File" << std::endl;
        this->curWriteFile.close();
    }
}

int FileOpt::writeFileAtOnce(std::string fileName, char* content) {
    stripRoot(fileName);

    std::ofstream myFile(fileName.c_str(), std::ios::out|std::ios::binary);
    if(!myFile) {
        std::cerr << "Cannot open output file '" << fileName << "'" << std::endl;
        return (EXIT_FAILURE);
    }
    myFile << content;
    myFile.close();
    return (EXIT_SUCCESS);
}

//CMD
std::string FileOpt::getParentDir() {
    if( this->completePath.size() <= this->rootDirCnt ) {
        std::cerr << "ERROR:beyond to root dir！" << std::endl;
        return this->rootDir;
    } else {
        auto lastDirIter=this->completePath.rbegin();
        return *(++lastDirIter);
    }
}

std::string FileOpt::getCurDir(bool showRootPath) {
    std::string fullpath = "";
    int cnt=0;
    for ( auto iter = this->completePath.begin(); iter != this->completePath.end(); ++iter) {
        if( cnt > 0 ) { cnt--; continue; }
        if ( iter == this->completePath.begin() ) {
            if ( !showRootPath ) { 
                fullpath.append(ROOT_DIR_MASK);
                cnt=this->rootDirCnt-1;
            } else fullpath.append(*(iter));
        } else fullpath.append(*(iter));
        fullpath.append(SLASH);
    }
    return fullpath;
}

bool FileOpt::changeDir(const std::string& newPath) {
    //处理..
    if ( (newPath.compare("..") == 0) || (newPath.compare("../") == 0) ) {
        if ( this->completePath.size() <= this->rootDirCnt ) { 
            std::cerr << "Error: beyond root dir!" << std::endl;
            return (EXIT_FAILURE);
        } else {
            this->completePath.emplace_back(); 
            return (EXIT_SUCCESS);
        }
    }

    //处理.
    if ( (newPath.compare(".") == 0) || (newPath.compare("./") == 0)) {
        std::cout << "Change to same dir!" << std::endl;
        return (EXIT_SUCCESS);
    }
    
    std::string path=this->getCurDir().append(newPath);
    if ( this->dirCanBeOpenend(path) ) {
        for( auto& dir : this->detachPath(newPath) )
            this->completePath.emplace_back(dir);
        return (EXIT_SUCCESS);
    } else {
        std::cerr << "ERROR: Open Dir " << path << " Failed！" << std::endl;
        return (EXIT_FAILURE);
    }
}

void FileOpt::browse(const std::string& dir, std::vector<std::string> &dirs, std::vector<std::string> &files) {
    std::string curPath="";
    curPath=this->getCurDir(true);
    if( dir.compare("./") != 0  ) curPath+=dir;

    std::cout << "Opening Dir='" << curPath << "'" << std::endl;
    DIR *dp;
    struct dirent *dirp;
    if ( this->dirCanBeOpenend(curPath) ) {
        try {
            dp = opendir(curPath.c_str());
            while ( (dirp = readdir(dp) ) != NULL ) {
                if (((std::string)dirp->d_name).compare("..") == 0 && this->completePath.size() < 2) continue;
                if( dirp->d_type == DT_DIR ) dirs.emplace_back(std::string(dirp->d_name));    
                else files.push_back(std::string(dirp->d_name));
            }
            closedir(dp);
        } catch (std::exception e) {
            std::cerr << "Error (" << e.what() << ") opening '" << curPath << "'" << std::endl;
        }
    } else
        std::cout << "Error: Dir '" << dir << "' could not be opened!" << std::endl;
}

//检查有效性
bool FileOpt::dirCanBeOpenend(const std::string& dir) {
    bool canBeOpened = false;
    DIR *dp=opendir(dir.c_str());
    if( dp != NULL ) canBeOpened=true;
    closedir(dp);
    return canBeOpened;
}

bool FileOpt::dirIsBelowServerRoot(std::string dirName) {
    return ((dirName.compare("../") == 0) && (this->completePath.size() < 2));
}