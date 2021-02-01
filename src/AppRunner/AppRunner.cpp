//
// Created by danny on 2021/1/19.
//

#include "AppRunner.h"
#include<unistd.h>
#include<cstring>
#include<wait.h>
#include<iostream>
#include<assert.h>
#include<fcntl.h>
#include"../util/SystemException.h"
#include"../util/WrappedFunc.h"
AppRunner::AppRunner(const std::string& path,std::vector<std::string> args,std::vector<std::string> env,std::string cwd)
:path(path),_pid(-1),fromApp{-1,-1},toApp{-1,-1},args(args),env(env),cwd(cwd),envInherit(true)
{

}
void AppRunner::start()
{
    if(this->stdio()==std::pair<int ,int >(-1,-1))
    {
        throw SystemException(9);
    }
    auto pid = fork();
    if(pid<0)
    {
        throw SystemException();
    }
    if(pid == 0)
    {
        //sub proc
        if(dup2(toApp[0],STDIN_FILENO)<0||dup2(fromApp[1],STDOUT_FILENO)<0)
        {
            exit(-1);
        }
        auto argvs = new char * [args.size()+1]{};
        char **envs;
        if(!this->envInherit)
        {
            envs = new char *[env.size()+1]{};
            for(int i = 0;i<env.size();i++)
            {
                envs[i] = new char[env[i].length()+1]{};
                memcpy(envs[i],env[i].c_str(),env[i].length());
            }
        }
        else
        {
            envs = environ;
        }
        for(int i = 0;i<args.size();i++)
        {
            argvs[i] = new char[args[i].length()+1];
            memcpy(argvs[i],args[i].c_str(),args[i].length());
        }
        chdir(cwd.c_str());
        execve(path.c_str(), argvs, envs);
        exit(127); //子进程正常执行则不会执行此语句
    }
    else
    {
        //origin proc
        this->_pid = pid;
    }
}

bool AppRunner::isRunning() {
    if (this->_pid == -1)
    {
        return false;
    }
    int32_t stat_loc;
    int res = waitpid(this->_pid, &stat_loc, WNOHANG);
    if (res == -1) {
        this->_pid = -1;
        this->_exitStat = 0;
    }
    if (res == 0) {
        return true;
    }
    else
    {
        this->_pid = -1;
        this->_exitStat = stat_loc;
        return false;
    }
}

pid_t AppRunner::pid() const
{
    return this->_pid;
}

int32_t AppRunner::exitStat()
{
    this->isRunning();
    return this->_exitStat;
}

void AppRunner::sendSig(int32_t signo)
{
    if(this->isRunning())
    {
        kill(this->_pid,signo);
    }
}

void AppRunner::wait()
{
    if (this->_pid == -1)
    {
        return;
    }
    int32_t stat_loc;
    int res = waitpid(this->_pid, &stat_loc, 0);
    if (res == -1) {
        this->_pid = -1;
        this->_exitStat = 0;
    }
    if (res == 0) {
        return;
    }
    else
    {
        this->_pid = -1;
        this->_exitStat = stat_loc;
        return;
    }
}

std::pair<int, int> AppRunner::stdio()
{
    return {this->fromApp[0],this->toApp[1]};
}

AppRunner::~AppRunner()
{
    if(this->stdio()==std::pair<int ,int >(-1,-1))
    {
        return;
    }
    if(this->isRunning())
    {
        this->wait();
    }
    close(fromApp[0]);
    close(fromApp[1]);
    close(toApp[0]);
    close(toApp[1]);
}

void AppRunner::init()
{
    if(fromApp[0] != -1)
    {
        return;
    }
    if(pipe(this->fromApp)==-1)
    {
        throw SystemException();
    }
    if(pipe(this->toApp)==-1)
    {
        close(fromApp[0]);
        close(fromApp[1]);
        this->fromApp[0] = this->fromApp[1] = -1;
        this->toApp[0] =  this->toApp[1] = -1;
        throw SystemException();
    }
}

AppRunner::AppRunner(const std::string &path, std::vector<std::string> args):
path(path),_pid(-1),fromApp{-1,-1},toApp{-1,-1},args(args),cwd("./"),envInherit(true)
{

}

AppRunner::AppRunner(const std::string &path, std::vector<std::string> args, std::string cwd)
:path(path),_pid(-1),fromApp{-1,-1},toApp{-1,-1},args(args),cwd(cwd),envInherit(true)
{


}

void AppRunner::clearPipe()
{
    ::clearPipe(this->fromApp[0]);
    ::clearPipe(this->toApp[0]);
}

std::pair<int *, int *> AppRunner::_pipes() {
    return {fromApp,toApp};
}

int AppRunner::write(const std::string &data)
{
    return ::write(this->toApp[1],data.c_str(),data.length());

}

int AppRunner::readLine(std::string &buffer, int timeout)
{
    int fd = this->fromApp[0];
    char temp;
    if(setNonblock(fd)==-1)
    {
        return -1;
    }
    while(true)
    {
        int res = ::read(fd,&temp,1);
        if(res < 0)
        {
            if(errno==EWOULDBLOCK)
            {
                if(haveData(timeout))
                {
                    continue;
                }
                else
                {
                    setBlock(fd);
                    return 0;
                }
            }
            else
            {
                setBlock(fd);
                return res;
            }
        }
        buffer.push_back(temp);
        if(temp =='\n')
        {
            break;
        }
    }
    setBlock(fd);
    return buffer.length();
}

int AppRunner::read(std::string& buffer, int n)
{
    buffer.resize(n);
    int res = ::read(this->fromApp[0],(char*)buffer.data(),n);
    if(res>=0)buffer.resize(res);
    return res;
}

int AppRunner::read(char *buffer, int n)
{
    int res = ::read(this->fromApp[0],buffer,n);
    return res;
}

bool AppRunner::haveData(int timeout)
{
    int fd = this->fromApp[0];
    fd_set fds;
    again:
    FD_ZERO(&fds);
    FD_SET(fd,&fds);
    timeval interval;
    interval.tv_sec = timeout;
    interval.tv_usec = 0;
    int nfd = select(fd+1,&fds,NULL,NULL,&interval);
    if(nfd <0)
    {
        if(errno ==EINTR)goto again;
        assert(nfd>=0);
    }
    return nfd==1;
}

int AppRunner::setBlock(bool block) {
    int flags = fcntl(this->fromApp[0], F_GETFL, 0);
    if (flags < 0) {
        return -1;
    }
    if(block)flags &=~O_NONBLOCK;
    else flags |=O_NONBLOCK;
    if (fcntl(this->fromApp[0], F_SETFL, flags) < 0) {
        return -1;
    }
    return 0;
}

