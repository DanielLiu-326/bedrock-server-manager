//
// Created by danny on 2021/1/19.
//

#ifndef BEDROCKMGR_APPRUNNER_H
#define BEDROCKMGR_APPRUNNER_H

#include<string>
#include<vector>
#include<map>
#include<mutex>
#define MAX_PATH 512
class AppRunner
{
public:
    //constructor & destructor
    AppRunner(const std::string& path,std::vector<std::string> args,std::vector<std::string> env,std::string cwd);
    AppRunner(const std::string& path,std::vector<std::string> args);
    AppRunner(const std::string& path,std::vector<std::string> args,std::string cwd);
    ~AppRunner();

    int                 write(const std::string& write);
    int                 readLine(std::string& buffer,int timeout);
    int                 read(std::string& buffer,int n);
    int                 read(char* buffer,int n);
    bool                haveData(int timeout);
    void                clearPipe();                       //this function should called when app is not running

    void                init();
    void                start();
    std::pair<int,int>  stdio();
    void                sendSig(int32_t signo);
    bool                isRunning();
    pid_t               pid() const;
    void                wait();                            //wait the process until it stopped
    int32_t             exitStat();                        //get the exit stat;
    std::pair<int *,int *> _pipes();
private:
    int                setBlock(bool block);
    //running arguments
    std::string                 path;
    std::vector<std::string>    args;
    std::vector<std::string>    env;
    bool                        envInherit;
    std::string                 cwd;

    //ipcs
    int fromApp[2];
    int toApp[2];

    //running states;
    int     _pid;
    int32_t _exitStat;
};


#endif //BEDROCKMGR_APPRUNNER_H
