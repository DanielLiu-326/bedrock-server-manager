//
// Created by danny on 2021/1/20.
//

#include "WrappedFunc.h"
#include<signal.h>
#include<unistd.h>
#include<fcntl.h>
#include<array>
#include<assert.h>
int sigBlock(int signo)
{
    sigset_t  set;
    sigemptyset(&set);
    sigaddset(&set,SIGCHLD);
    return sigprocmask(SIG_BLOCK,&set,0);
}
int sigUnblock(int signo)
{
    sigset_t  set;
    sigemptyset(&set);
    sigaddset(&set,SIGCHLD);
    return sigprocmask(SIG_UNBLOCK,&set,0);
}
std::vector<std::string> getEnviron()
{
    std::vector<std::string> ret;
    for(int i = 0;environ[i];i++)
    {
        ret.emplace_back(environ[i]);
    }
    return ret;
}
int setNonblock(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0) {
        return -1;
    }
    flags |= O_NONBLOCK;
    if (fcntl(fd, F_SETFL, flags) < 0) {
        return -1;
    }
    return 0;
}
int setBlock(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0) {
        return -1;
    }
    flags &= !O_NONBLOCK;
    if (fcntl(fd, F_SETFL, flags) < 0) {
        return -1;
    }
    return 0;
}
int clearPipe(int readFd)
{
    int res = setNonblock(readFd);
    if(res<0)
    {
        return -1;
    }
    char buffer[64];

    while(read(readFd,buffer,sizeof(buffer))>0);

    setBlock(readFd);
    return 0;
}
std::vector<std::string> splitStr(const std::string & str,char split)
{
    std::vector<std::string> ret;
    int last = 0;
    for(int i = 0;i<str.length();i++)
    {
        if(str[i] == split)
        {
            ret.push_back(str.substr(last,i-last));
            last = i + 1;
        }
    }
    ret.push_back(str.substr(last));
    return ret;
}
int copyFile(std::string from ,std::string to)
{
    char buff[1024];
    int len;
    int fd,fd2;
    fd = open(from.c_str(),O_RDONLY);
    if(fd<0)return -1;
    fd2 = open(to.c_str(),O_WRONLY|O_CREAT);
    if(fd2<0)return -1;
    while(true)
    {
        len = read(fd,buff,1024);
        if(len<0)
        {
            close(fd);
            close(fd2);
            return -1;
        }
        if(len == 0)
        {
            break;
        }
        write(fd2,buff,len);
    }
    close(fd);
    close(fd2);
    return 0;
}

std::string trimStr(const std::string& str)
{
    int from = str.find_first_not_of(' ');
    from = (from == str.npos?0:from);
    int to = str.find_last_not_of(' ');
    to = (to== str.npos?0:to);
    return str.substr(from,to - from+1);
}