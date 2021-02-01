//
// Created by danny on 2021/1/25.
//

#ifndef BEDROCKMGR_WEBAPIMAIN_H
#define BEDROCKMGR_WEBAPIMAIN_H
#include <cppcms/application.h>
#include <cppcms/applications_pool.h>
#include <cppcms/service.h>
#include <cppcms/http_response.h>
#include<cppcms/url_dispatcher.h>
#include<cppcms/url_mapper.h>
#include<cppcms/http_request.h>
#include<cppcms/rpc_json.h>
#include<cppcms/json.h>
#include"ServerMgr.h"
#include<set>
class WebApiMain:public cppcms::rpc::json_rpc_server
{
public:
    explicit WebApiMain(cppcms::service &srv,ServerMgr *serverMgr,std::string pass);
    void whitelist_list(std::string pass);
    void whitelist_ar(std::string player,bool operation , std::string pass);
    void command(std::string  command,std::string pass);
    void op(std::string name,bool operation,std::string pass);
    void kick(std::string name,std::string reason,std::string pass);
    void kill(std::string name,std::string pass);
    void gamemode(std::string name,int mode ,std::string pass);
    void permission_set(std::string name,int mode,std::string pass);
    void say(std::string words ,std::string pass);
    void list();
    void start(std::string pass);
    void stop(std::string pass);
private:
    std::string pass;
    ServerMgr* serverMgr;
};



#endif //BEDROCKMGR_WEBAPIMAIN_H
