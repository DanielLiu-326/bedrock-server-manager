//
// Created by danny on 2021/1/31.
//

#include "Logger.h"
#include<unistd.h>
#include<fcntl.h>
#include<iostream>
#include<sstream>
Logger::Logger(std::string filePath):fd(-1)
{
    remove(filePath.c_str());
    this->fd= open(filePath.c_str(),O_WRONLY|O_CREAT|O_TRUNC);
    if(fd<0)
    {
        throw SystemException();
    }
}

Logger::~Logger()
{
    if(this->fd>0)close(this->fd);

}

Logger &Logger::operator<<(const std::string &what) {
    std::string temp = what;
    if(what.back()!='\n')
    {
        temp += '\n';
    }
    int writeLen = write(this->fd,temp.c_str(),temp.length());
    if(writeLen<0)
    {
        throw SystemException();
    }
    return *this;
}