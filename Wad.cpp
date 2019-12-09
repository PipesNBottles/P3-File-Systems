#include "Wad.h"

Wad* Wad::loadWad(const std::string &path)
{
    static Wad* wad = new Wad(path);
    return wad;
}

int Wad::getDirectory(const std::string &path, std::vector<std::string> *directory)
{
    std::size_t location = path.rfind('/');
    std::string filePath = path.substr(0,location+1);
    std::string file = path.substr(location+1);
    if (this->isDirectory(path))
    {
        for(auto it = this ->files.begin(); it != this->files.end(); it++)
        {
            if(it->first.compare(filePath) == 0)
            {
                directory->push_back(it->second.name);
            }
        }
        int totalFiles = directory->size();
        return totalFiles;
    }
    
    return -1;
}

int Wad::getContents(const std::string &path, char* buffer, int length, int offset)
{
    std::size_t location = path.rfind('/');
    std::string filePath = path.substr(0,location+1);
    std::string file = path.substr(location+1);
    std::string b2 = "";
    int bytesCopied = 0;
    for(auto it = this ->files.begin(); it != this->files.end(); it++)
    {
        if(it->first.compare(filePath) == 0)
        {
            if (it->second.name.compare(file) == 0)
            {
                for(int i = offset ; i != length; i++)
                {
                    b2 += it->second.contents.at(i);
                    bytesCopied++;
                }
                memcpy(buffer,b2.c_str(),bytesCopied);
                return bytesCopied;
            }
                    
        }
    }
    return -1;
}

int Wad::getSize(const std::string &path)
{
    std::size_t location = path.rfind('/');
    std::string filePath = path.substr(0,location+1);
    std::string file = path.substr(location+1);
    for(std::multimap<std::string, struct contents>::iterator it = this ->files.begin(); it != this->files.end(); it++)
    {
        if(it->first.compare(filePath) == 0)
        {
            if (it->second.name.compare(file) == 0)
            {
                return it->second.size;
            }
                    
        }
    }
    return -1;
}

bool Wad::isContent(const std::string &path)
{
    std::size_t location = path.rfind('/');
    std::string filePath = path.substr(0,location+1);
    std::string file = path.substr(location+1);
    for(std::multimap<std::string, struct contents>::iterator it = this ->files.begin(); it != this->files.end(); it++)
    {
        if(it->first.compare(filePath) == 0)
        {
            if (it->second.name.compare(file) == 0)
            {
                return true;
            }
                    
        }
    }
    return false;
}

bool Wad::isDirectory(const std::string &path)
{
    bool exists = this->allPaths.find(path) != this->allPaths.end();
    if (exists)
    {
        return true;
    }
    return false;   

}

std::string Wad::getMagic()
{
    std::string toss;
    for (int i = 0; i < 5; i++)
    {
        toss += this -> magic[i];
    }
    
    toss.erase(std::remove(toss.begin(),toss.end(),'\000'),toss.end());
    return toss;
}

Wad::Wad(const std::string &path)
{
    this->readHeader(path.c_str());
    this->readlumps(path.c_str());
}

void Wad::printall()
{
    std::cout << "magic is(hex): " << std::hex << this->getMagic() << std::endl;
    std::cout << "num flags is(hex): " << std::hex << this -> descriptors << std::endl;
    std::cout << "address to start lump search(hex): " << std::hex << this -> firstAddress << std::endl;
    for (std::multimap<std::string,struct contents>::iterator it=this->files.begin(); it!=this->files.end(); ++it)
    {
        std::cout << "Directory Name: " << (*it).first << " File Size: " << (*it).second.size << std::endl;
        std::cout << "File Name: "<<(*it).second.name << std::endl;
        std::cout << "File Contents: " <<(*it).second.contents << std::endl;
    }

}

Wad::~Wad()
{
}

void Wad::readHeader(const char * path)
{
    std::ifstream infile;
    infile.open(path, std::ios::binary);

    if(!infile.is_open()) exit(1);

    infile.seekg(0, std::ios::beg);

    for(int i = 0; i != 3; i++){
        uint8_t buffer[4] = {0};
        unsigned int result = buffer[0];
        infile.read((char*)buffer, sizeof(buffer));
        result = (result << 8) | buffer[0];
        result = (result << 8) | buffer[1];
        result = (result << 8) | buffer[2];
        result = (result << 8) | buffer[3];
        result = ntohl(result);
        switch (i)
        {
        case 0:
            this -> magic[0] = (char) buffer[0];
            this -> magic[1] = (char) buffer[1];
            this -> magic[2] = (char) buffer[2];
            this -> magic[3] = (char) buffer[3];
            this -> magic[4] = '\0';
            break;
        case 1:
            this -> descriptors = result;
            break;
        case 2:
            this -> firstAddress = result;
        default:
            break;
        }

    }
    infile.close();

}

void Wad::readlumps(const char* path)
{
    uint8_t temp;
    uint8_t buffer[4] = {0};
    uint8_t buffer2[8];
    unsigned int result = buffer[0];
    int count = 0;
    int lumpOffset = 0;
    int lumpSize = 0;
    int mapElements = 0;
    char lumpName[9];
    std::ifstream infile;
    std::string toss, _end, _start, param1;
    std::stack<std::string> stack;
    infile.open(path, std::ios::binary);

    _end = "_END";
    _start = "_START";
    param1 = "vuoto";

    if(!infile.is_open()) exit(1);

    infile.seekg(this->firstAddress, std::ios::beg);

    while (!infile.eof())
    {
        switch (count)
        {
        case 0:
            infile.read((char*)buffer, sizeof(buffer));
            for (int j = 3; j >= 0; j--)
            {
                result = (result << 8) | buffer[j];
            }
            
            lumpOffset = result;
            result = 0;
            count++;
            break;
        case 1:
            infile.read((char*)buffer, sizeof(buffer));
            for (int j = 3; j >= 0; j--)
            {
                result = (result << 8) | buffer[j];
            }
            
            lumpSize = result;
            result = 0;
            count++;
            break;
        case 2:
            infile.read((char*)buffer2, sizeof(buffer2));
            for (size_t j = 0; j != 8; j++)
            {
                lumpName[j] = (char) buffer2[j];
                toss += lumpName[j];
            }
            toss.erase(std::remove(toss.begin(),toss.end(),'\000'),toss.end());
            memset(lumpName,0,sizeof(lumpName));
            count++;
            break;
        default:
            this -> allPaths.insert(this->currentPath);
            
            count = 0;
            if(lumpSize == 0)
            {
                if(toss[0] == 'E' && toss[2] == 'M')
                {
                    stack.push(toss);
                    this -> currentPath += toss; 
                    this -> currentPath += "/";
                }

                else if (toss.find(_start) != std::string::npos)
                {
                    stack.push(toss);
                    std::string toss2 = toss.substr(0,toss.find(_start));
                    this -> currentPath += toss2;
                    this -> currentPath += "/";
                }

                else if (toss.find(_end) != std::string::npos)
                {
                    stack.pop();
                    this -> currentPath = this -> currentPath.substr(0,this->currentPath.find(toss.substr(0,toss.find(_end))));
                }
            }

            else
            {
                if (!stack.empty())
                {
                    param1 = std::string(stack.top());
                }

                if (param1.at(0) == 'E' && param1.at(2) == 'M')
                {
                    mapElements++;
                    this -> buildData(path, lumpOffset,lumpSize,toss);
                }

                else
                {
                    this -> buildData(path, lumpOffset,lumpSize,toss);
                }

                if (mapElements == 10)
                {
                    std::string toss2 = stack.top();
                    this -> currentPath = this->currentPath.substr(0,this->currentPath.find("E"));
                    stack.pop();
                    mapElements = 0;
                }
            }

            toss.clear();
            lumpOffset = 0;
            lumpSize = 0;
            param1 = "vuoto";
        }
    }
    infile.close();
    return;

}

void Wad::buildData(const char* path, int lumpOffset, int lumpSize, std::string lumpName)
{
    uint8_t temp;
    uint8_t buffer[lumpSize] = {0};
    unsigned char words[lumpSize];
    unsigned int result = buffer[0];
    std::ifstream infile;
    infile.open(path, std::ios::binary);
    if(!infile.is_open()) exit(1);
    infile.seekg(lumpOffset, std::ios::beg);
    infile.read((char*)buffer, sizeof(buffer));
    contents file;
    file.name = lumpName;
    file.size = lumpSize;
    for (int i = 0; i != lumpSize; i++)
    {
        words[i] = (char) buffer[i];
    }
    file.contents = std::string(reinterpret_cast<char*>(words), sizeof(words)/sizeof(words[0]));
    std::pair<std::string,struct contents> pair1(this->currentPath,file);
    this ->files.insert(pair1);
    infile.close(); 

    return;
};

int main(int argc, char* argv[]){
  Wad* wad = Wad::loadWad("sample1.wad");
  std::vector<std::string> entries;
  int level = wad->getDirectory("/",&entries);
  return 0;
}