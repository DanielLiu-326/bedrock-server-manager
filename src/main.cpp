#include <iostream>
#include<unistd.h>
#include<fstream>
#include"WebApiMain.h"
#include"./util/WrappedFunc.h"
class EventHandler:public ServerEvent
{
public:
    void onPlayerDisconnected(std::string name) override
    {
        std::cout<<"[Event]"<<"player \""<<name<<"\" disconnected"<<std::endl;
    }
    void onPlayerConnected(std::string name,std::string xuid)override
    {
        std::cout<<"[Event]"<<"player \""<<name<<"\",xuid:"<<xuid<<" connected"<<std::endl;
    }
    void onServerStarted()override
    {
        std::cout<<"[Event]server started"<<std::endl;
    }
    void onServerStopped(int32_t stat)override
    {
        std::cout<<"[Event]server stopped"<<std::endl;
    }
    void Timer()override
    {

    }
};
void mainCirculate(ServerMgr & mgr , cppcms::service& srv);
int main(int argc,char *argv[]) {
    char *_argv[] = {"","-c","./config.json", nullptr};
    nlohmann::json js;
    std::ifstream stream("./config.json");
    stream>>js;
    stream.close();
    std::string password = js["password"];
    EventHandler eventHandler;
    ServerMgr serverMgr("./core",&eventHandler);
    serverMgr.startServer();
    std::thread mgrCirculate(&ServerMgr::circulate,&serverMgr);
    cppcms::service srv(3, _argv);
    srv.applications_pool().mount(cppcms::applications_factory<WebApiMain>(&serverMgr,password));
    std::thread apiCirculate(&cppcms::service::run,&srv);
    mainCirculate(serverMgr,srv);
    mgrCirculate.join();
    apiCirculate.join();
    return 0;
}
void mainCirculate(ServerMgr & mgr , cppcms::service& srv)
{
    while(true)
    {
        char inputBuffer[1000];
        std::cin.getline(inputBuffer,sizeof(inputBuffer));
        std::string input = inputBuffer;
        input = trimStr(input);
        int err;
        if(input == "stop")
        {
            err= mgr.stopServer();
        }
        else if(input =="start")
        {
            err=mgr.startServer();
        }
        else if(input == "quit")
        {
            mgr.stopServer();
            srv.shutdown();
            mgr.interrupt();
            return ;
        }else if(splitStr(input,' ')[0]=="mc")
        {
            std::string res;
            err = mgr.command(input.substr(input.find("mc")+2),res);
            if(!err)
            {
                std::cout<<res<<std::endl;
            }
        }
        if(err)
        {
            std::cout<<"err:"<<strResult(Result(err))<<std::endl;
        }
    }
}