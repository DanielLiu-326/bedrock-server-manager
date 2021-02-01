//
// Created by danny on 2021/1/21.
//

#include "ServerMgr.h"
#include"util/WrappedFunc.h"
#include<unistd.h>
#include<fcntl.h>
#include<regex>
#include<fstream>
#include<iostream>
#include<sstream>
#include<fstream>
#include"nlohmann/json.hpp"
std::string strResult(Result res)
{
    switch (res) {
        case OK :                   return "OK";
        case SYNTAX_ERROR:          return "SYNTAX_ERROR";
        case NO_TARGET:             return "NO_TARGET";
        case AUTH_ERROR:            return "AUTH_ERROR";
        case SERVER_NOT_RUNNING:    return "SERVER_NOT_RUNNING";
        case SERVER_IS_RUNNING:     return "SERVER_IS_RUNNING";
        case COMMAND_INVALID:       return "COMMAND_INVALID";
        case GAMEMODE_INVALIDE:     return "GAMEMODE_INVALIDE";
        case NO_RESPONSE:           return "NO_RESPONSE";
        default:return "UNDEFINED_ERROR";
    }
}
ServerMgr::ServerMgr(const std::string &core,ServerEvent *eventHandler)
: core(core),_stat(ServerStat::STOPPED),eventHandler(eventHandler),interruptFlag(false),logger(core+"/log.txt")
{
    this->server = new AppRunner("./bedrock_server",{"./bedrock_server"},core);
    this->server->init();
    this->loadXuids();
}

int ServerMgr::startServer()
{
    if(this->server->isRunning())
    {
        return SERVER_IS_RUNNING;
    }
    this->server->clearPipe();
    this->server->start();
    return 0;
}

int ServerMgr::stopServer()
{
    if(server->isRunning())
    {
        this->sendCmd("stop\nstop\n");
        //this is a feature(bug) of bds. when server crashed , you have to input stop double time to quit it
        this->server->wait();
    }
    else
    {
        return SERVER_NOT_RUNNING;
    }
    return 0;
}


void ServerMgr::sendCmd(std::string cmd)
{
    write(server->stdio().second, cmd.c_str(), cmd.length());
}

void ServerMgr::circulate()
{
    this->interruptFlag = false;
    bool running = this->server->isRunning();
    char buffer[1024];
    while(true)
    {
        waitEvents();
        {
            std::lock_guard<typeof(this->interruptLock)> lockGuard(this->interruptLock);
            if(this->interruptFlag)
            {
                interruptFlag = false;
                break;
            }
            std::lock_guard<typeof(this->ioLock)> guard(this->ioLock);
            //server stat running -> stopped
            if (this->server->isRunning() != running && running)
            {
                this->eventHandler->onServerStopped(this->server->exitStat());
            }
            running = this->server->isRunning();
            this->eventHandler->Timer();//call timer;
        }
        handleEvents();
    }
}

void ServerMgr::interrupt()
{
    std::lock_guard<typeof(this->interruptLock)> guard(this->interruptLock);
    this->interruptFlag = true;

}

int ServerMgr::copyWorld(std::string levelName,std::string newName)
{
    std::lock_guard<typeof(this->interruptLock)> guard(this->interruptLock);
    std::stringstream cmd;
    std::stringstream newPath;
    newPath<<"\""<<core<<"/worlds/"<<newName<<"/\"";
    cmd<<"rm -rf "<<newPath.str();
    auto fp = popen(cmd.str().c_str(),"r");
    if(!fp)
    {
        return -1;
    }

    cmd.clear();
    cmd<<"mkdir "<<newPath.str();
    fp = popen(cmd.str().c_str(),"r");
    if(!fp)
    {
        return -1;
    }
    auto rv = pclose(fp);

    cmd.clear();
    cmd<<"cp -r "
    <<"\""<<core<<"/worlds/"<<levelName<<"/\"*"
    <<" "
    <<newPath.str();
    fp = popen(cmd.str().c_str(),"r");
    rv = pclose(fp);
    if(!WIFEXITED(rv))
    {
        return -1;
    }
    if(WEXITSTATUS(rv)!=0) {
        return WEXITSTATUS(rv);
    }
    int fd = open((newPath.str()+"levelname.txt").c_str(),O_WRONLY);
    if(fd<0)
    {
        return -1;
    }
    write(fd,newName.c_str(),newName.length());
    close(fd);
    return 0;
}

int ServerMgr::kill()
{
    if(this->server->isRunning())
    {
        this->server->sendSig(9);//kill sig;
        return 0;
    }
    else
    {
        return SERVER_NOT_RUNNING;
    }
}


void ServerMgr::handleEvents()
{
    std::lock_guard<typeof(this->ioLock)> guard(this->ioLock);
    while(!eventQueue.empty())
    {
        auto ev = eventQueue.front();
        eventQueue.pop();
        if(ev.find("[INFO] Player connected:")==0)
        {
            std::string xuid,name;
            //PlayerConnected/ [INFO] Player disconnected: DannyLH326, xuid: 2535448334573239
            int from = ev.find_first_of(":");
            int to = ev.find_last_of(',');
            name = ev.substr(from+2,to-from-2);
            from = ev.find_last_of(":");
            xuid = std::to_string(std::stoull(ev.substr(from+1)));
            this->xuids[name] = xuid;
            eventHandler->onPlayerConnected(name,xuid);
            continue;
        }
        if(ev.find("[INFO] Server started.")==0)
        {
            //Server Started;
            eventHandler->onServerStarted();
            continue;
        }
        if(ev.find("[INFO] Player disconnected") == 0)
        {
            //[INFO] Player disconnected: DannyLH326, xuid: 2535448334573239
            int from = ev.find_first_of(":");
            int to = ev.find_last_of(',');
            eventHandler->onPlayerDisconnected(ev.substr(from+2,to-from-2));
            continue;
        }
    }
}

void ServerMgr::waitEvents() {

    if(server->haveData(1))//wait
    {
        //between wait and lock,data may be changed by other thread;
        std::lock_guard<typeof(this->ioLock)> guard(this->ioLock);
        if (server->haveData(0))//check after lock
        {
            std::string buffer;
            server->readLine(buffer,1);
            logger<<buffer;
            if (buffer[0] == '[') {
                this->eventQueue.push(buffer);
            }
        }
    }
}

int ServerMgr::whitelist_op(std::string name, bool op) {
    trimStr(name);
    std::string res;
    std::stringstream stream;
    if(op)
    {
        stream<<"whitelist add \""<<name<<"\"";
    }
    else
    {
        stream<<"whitelist remove \""<<name<<"\"";
    }
    auto re = this->command(stream.str(),res);
    if(re)return re;
    if(res.find("Syntax error:")==0)return SYNTAX_ERROR;
    return 0;
}

int ServerMgr::op(bool isOperator, std::string name) {
    std::stringstream cmd;
    if (isOperator)cmd << "op ";
    else cmd << "deop ";
    cmd<<"\""<<name<<"\"";
    std::string res;
    auto re = this->command(cmd.str(),res);
    if(re)return re;
    if(res.find("Syntax error:")==0)
    {
        return SYNTAX_ERROR;
    }
    if(res.find("Could not op")==0)
    {
        return 0;
    }
    if(res.find("No targets matched selector")==0)
    {
        return NO_TARGET;
    }
    return 0;
}

int ServerMgr::list(int &max, std::vector<std::string> &ret) {
    std::lock_guard<typeof(this->ioLock)>lock(this->ioLock);
    std::string res;
    auto err = this->command("list",res);
    if(err)return err;
    //There are 1/10 players online:
    max = std::stoi(res.substr(12));
    res = res.substr(res.find_first_of('\n')+1);
    res.pop_back();
    ret = splitStr(static_cast<std::string &&>(res), ',');
    if(ret.back()=="")ret.pop_back();
    return 0;
}

int ServerMgr::whitelist(std::vector<std::string> &ret)
{
    //###* {"command":"whitelist","result":[{"name":"yourname326"},{"name":"danny326"},
    std::lock_guard<typeof(this->ioLock)>lock(this->ioLock);
    std::string res;
    auto re = this->command("whitelist list",res);
    if(re)return re;
    auto pos1 = res.find_first_of('{');
    auto pos2 = res.find_last_of('}');
    auto jsonStr = res.substr(pos1,pos2-pos1+1);
    nlohmann::json j = nlohmann::json::parse(jsonStr);
    for(auto x:j["result"])
    {
        ret.push_back(x["name"]);
    }
    return  0;
}

int ServerMgr::command(std::string cmd, std::string &ret) {
    if(cmd.find('\n')!=cmd.npos) {
        return COMMAND_INVALID;
    }
    logger<<cmd;
    cmd +="\n";
    std::lock_guard<typeof(this->ioLock)> lockGuard(this->ioLock);
    if(!this->server->isRunning())
    {
        return SERVER_NOT_RUNNING;
    }
    sendCmd(cmd);
    cmd = CMD_RES_SPLITER;
    cmd+="\n";
    sendCmd(cmd);
    while(true)
    {
        std::string buffer;
        int res = server->readLine(buffer,CMD_TIME_OUT);
        if(res==-1)
        {
            throw SystemException();
        }
        if(res==0)
        {
            return NO_RESPONSE;
        }
        if(buffer.find(CMD_RES_SPLITER)!=buffer.npos)
        {
            break;
        }
        logger<<buffer;
        if(buffer[0]=='[')//[info]
        {
            this->eventQueue.push(buffer);
            continue;
        }
        ret.append(buffer);
    }
    return 0;
}

int ServerMgr::kick(std::string name, std::string reason)
{
    std::lock_guard<typeof(this->ioLock)>lock(this->ioLock);
    std::stringstream  stream;
    stream<<"kick "<<"\""<<name<<"\" \""<<reason<<"\"";
    std::string result;
    this->command(stream.str(),result);
    if(result.find("No targets matched selector")==0)
    {
        return NO_TARGET;
    }
    return OK;
}

int ServerMgr::say(std::string words) {
    std::lock_guard<typeof(this->ioLock)>lock(this->ioLock);
    std::stringstream cmd;
    cmd<<"say "<<words;
    std::string res;
    auto err = this->command(cmd.str(),res);
    if(err)return err;
    if(res != "")return UNKNOWN;
    return 0;
}

int ServerMgr::kill(std::string player) {
    std::lock_guard<typeof(this->ioLock)>lock(this->ioLock);
    std::stringstream cmd;
    std::string res;
    cmd<<"kill \""<<player<<"\"";
    auto err = this->command(cmd.str(),res);
    if(err) return err;
    if(res.find("No targets matched selector")==0)return NO_TARGET;
    if(res.find("Syntax error:")==0)return SYNTAX_ERROR;
    return OK;
}

int ServerMgr::gamemode(std::string player, int mode) {
    std::lock_guard<typeof(this->ioLock)>lock(this->ioLock);
    std::stringstream cmd;
    cmd<<"gamemode "<<mode<<" \""<<player<<"\"";
    std::string res;
    int err = this->command(cmd.str(),res);
    if(err) return err;
    if(res.find("No targets matched selector")==0)return NO_TARGET;
    if(res.find("Syntax error:")==0)return SYNTAX_ERROR;
    if(res.find("Game mode ")==0)return GAMEMODE_INVALIDE;
    return OK;
}

int ServerMgr::loadXuids()
{
    nlohmann::json js;
    try
    {
        std::ifstream file(this->core+"/whitelist.json");
        file>>js;
        for(auto x: js){
            if(x.count("xuid")){
                this->xuids[x["name"]] = x["xuid"];
            }
        }
        file.close();
    }catch (...)
    {
        return -1;
    }
    return 0;
}

int ServerMgr::permission_set(std::string name, int permission)
{
    std::lock_guard<typeof(this->ioLock)>lock(this->ioLock);
    if(!this->xuids.count(name))
    {
        return NO_TARGET;
    }
    auto xuid = xuids[name];
    std::string permissionStr;
    switch (permission)
    {
        case 0:permissionStr = "visitor";
        break;
        case 1:permissionStr = "member";
        break;
        case 2:permissionStr = "operator";
        break;
        default:
            permissionStr = "visitor";
            break;
    }
    try
    {
        nlohmann::json js;
        std::ifstream file(this->core+"/permissions.json");
        file>>js;
        file.close();
        int count = 0,target = -1;
        for(auto x:js)
        {
            if(x["xuid"]==xuid)target = count;
            count++;
        }
        if(target>=0)
        {
            js[target]["permission"] = permissionStr;
        }
        else
        {
            js.push_back({{"permission",permissionStr},{"xuid",xuid}});
        }
        std::ofstream ofile(this->core+"/permissions.json",std::ios_base::out|std::ios_base::trunc);
        ofile<<js;
        ofile.close();
    }catch (...)
    {
        return UNKNOWN;
    }
    std::string res;
    auto err = this->command("permission reload",res);
    if(err)return err;
}
