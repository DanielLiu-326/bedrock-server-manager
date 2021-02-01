//
// Created by danny on 2021/1/21.
//

#ifndef BEDROCKMGR_SERVERMGR_H
#define BEDROCKMGR_SERVERMGR_H
#include<thread>
#include"util/SystemException.h"
#include<list>
#include<queue>
#include"AppRunner/AppRunner.h"
#include"nlohmann/json.hpp"
#include"Logger/Logger.h"
#define CMD_TIME_OUT 5
#define CMD_RES_SPLITER "723D505516E0C197E42A6BE3C0AF910E"
enum class ServerStat
{
    RUNNING,
    STOPPED,
    MAINTAINING,
};
enum class WhitelistOp
{
    add,
    remove,
};

class ServerEvent
{
public:
    virtual void onPlayerDisconnected(std::string name) = 0;
    virtual void onPlayerConnected(std::string name,std::string xuid) = 0;
    virtual void onServerStarted() = 0;
    virtual void onServerStopped(int32_t stat)=0;
    virtual void Timer() = 0;   //will call every second;

};
struct ServerError:public std::exception
{
    std::string why;
    ServerError(std::string str) :why(str)
    {

    }
    const char *what() const noexcept override
    {
        return this->why.c_str();
    }
};
enum Result
{
    OK= 0,
    SYNTAX_ERROR = 1,
    NO_TARGET = 2,
    AUTH_ERROR = 3,
    SERVER_NOT_RUNNING = 4,
    SERVER_IS_RUNNING = 5,
    COMMAND_INVALID = 6,
    GAMEMODE_INVALIDE = 7,
    NO_RESPONSE = 8,
    UNKNOWN = -1,

};
extern std::string strResult(Result res);
class ServerMgr
{
public:
    explicit ServerMgr(const std::string &core,ServerEvent*eventHandler);   //core是bedrock_server可执行文件所在目录
    void interrupt();       //打断事件循环

    int startServer();     //开启服务器
    int stopServer();      //关闭服务器
    int kill();            //杀死服务器进程

    int permission_set(std::string name ,int permission);   //设置玩家权限 (0:visitor,1:member,2:operator)
    int kick(std::string name,std::string reason);          //踢出玩家
    int say(std::string words);                             //发送服务器消息
    int kill(std::string player);                           //杀死玩家
    int gamemode(std::string player ,int mode);             //设置某个人的游戏模式
    int list(int &max,std::vector<std::string>& online);    //获取在线玩家
    int whitelist(std::vector<std::string>& list);          //获取白名单
    int whitelist_op(std::string name,bool op);             //添加或删除白名单
    int op(bool isOperator,std::string name);               //给予操作员权限
    int command(std::string cmd,std::string&result);        //执行命令
    void circulate();                                       //事件循环
private:
    Logger logger;
    std::map<std::string ,std::string> xuids;
    int loadXuids();                                       //尝试从whitelist.json文件中加载xuid
    void handleEvents();
    void waitEvents();
    int copyWorld(std::string levelName,std::string newName);
    std::recursive_mutex interruptLock;
    bool interruptFlag;
    ServerEvent*eventHandler;
    void sendCmd(std::string cmd);
    std::queue<std::string> eventQueue;
    std::recursive_mutex ioLock;
    std::string core;
    ServerStat _stat;
    AppRunner* server;

};



#endif //BEDROCKMGR_SERVERMGR_H
