//
// Created by danny on 2021/1/25.
//

#include "WebApiMain.h"
#include"util/md5.h"
#include<iostream>
WebApiMain::WebApiMain(cppcms::service &srv,ServerMgr *serverMgr,std::string pass)
: cppcms::rpc::json_rpc_server(srv),serverMgr(serverMgr),pass(pass)
{
    bind("command",cppcms::rpc::json_method(&WebApiMain::command,this),method_role);
    bind("stop",cppcms::rpc::json_method(&WebApiMain::stop,this),method_role);
    bind("start",cppcms::rpc::json_method(&WebApiMain::start,this),method_role);
    bind("list",cppcms::rpc::json_method(&WebApiMain::list,this),method_role);
    bind("whitelist_list",cppcms::rpc::json_method(&WebApiMain::whitelist_list,this),method_role);
    bind("whitelist_ar",cppcms::rpc::json_method(&WebApiMain::whitelist_ar,this),method_role);
    bind("op",cppcms::rpc::json_method(&WebApiMain::op,this),method_role);
    bind("kick",cppcms::rpc::json_method(&WebApiMain::kick,this),method_role);
    bind("say",cppcms::rpc::json_method(&WebApiMain::say,this),method_role);
    bind("kill",cppcms::rpc::json_method(&WebApiMain::kill,this),method_role);
    bind("gamemode",cppcms::rpc::json_method(&WebApiMain::gamemode,this),method_role);
    bind("permission_set",cppcms::rpc::json_method(&WebApiMain::permission_set,this),method_role);

}

void WebApiMain::command(std::string command,std::string pass)
{
    std::cout<<"[HTTP Call]command("<<command<<","<<pass<<")"<<std::endl;
    if(pass!=this->pass)
    {
        std::cout<<"[Reply]error:"<<AUTH_ERROR<<std::endl;
        return_error((int)AUTH_ERROR);
        return;
    }
    std::string res;
    auto err = this->serverMgr->command(command,res);
    if(err) {
        std::cout << "[Reply]error:" << strResult(Result(err)) << std::endl;
        return_error(err);
        return;
    }
    else
    {
        std::cout << "[Reply]" << res << std::endl;
        return_result(res);
        return;
    }

}

void WebApiMain::start(std::string pass) {
    std::cout << "[HTTP Call]start(" << pass << ")" << std::endl;
    if (pass != this->pass) {
        std::cout << "[Reply]error:" << strResult(AUTH_ERROR) << std::endl;
        return_error((int) AUTH_ERROR);
        return;
    }
    int err = this->serverMgr->startServer();
    if(err)
    {
        std::cout << "[Reply]error:" <<strResult(Result(err))<< std::endl;
        return_error((int) err);
    }
    else
    {
        std::cout << "[Reply]" <<"SUCCESS"<< std::endl;
        return_result("SUCCESS");
    }
}

void WebApiMain::stop(std::string pass)
{
    std::cout<<"[HTTP Call]stop(\""<<pass<<"\")"<<std::endl;
    if(pass!=this->pass){
        std::cout<<"[Reply]error:"<<AUTH_ERROR<<std::endl;
        return_error((int )AUTH_ERROR);
        return;
    }
    auto err = this->serverMgr->stopServer();
    if(err){
        std::cout<<"[Reply]error:"<<strResult((Result)err)<<std::endl;
        return_error(int(err));
    }
    else {
        std::cout << "[Reply]" << "SUCCESS" << std::endl;
        return_result("SUCCESS");
    }
}

void WebApiMain::list()
{
    std::cout<<"[HTTP Call]list()"<<std::endl;
    int max;
    std::vector<std::string> res;
    auto err= this->serverMgr->list(max,res);
    if(err){
        std::cout<<"[Reply]error:"<<strResult(Result(err))<<std::endl;
        return_error((int)err);
    }
    else{
        cppcms::json::object ret;
        ret["max"] = max;
        ret["online"] = res;
        std::cout<<"[Reply]"<<ret<<std::endl;
        return_result(ret);
    }
}

void WebApiMain::whitelist_list(std::string pass)
{
    std::cout<<"[HTTP Call]whitelist_list(\""<<pass<<"\")"<<std::endl;
    if(pass!=this->pass)
    {
        std::cout<<"[Reply]error:"<<strResult(AUTH_ERROR)<<std::endl;
        return_error((int)AUTH_ERROR);
        return;
    }
    std::vector<std::string> res;
    auto err = this->serverMgr->whitelist(res);
    cppcms::json::array ret(res.begin(),res.end());
    if(err)
    {
        std::cout<<"[Reply]error:"<<strResult((Result)err)<<std::endl;
        return_error((int)err);
    }
    else
    {
        std::cout<<"[Reply]"<<ret<<std::endl;
        return_result(ret);
    }
}

void WebApiMain::whitelist_ar(std::string player, bool operation, std::string pass) {
    std::cout<<"[HTTP Call]whitelist_ar(\""<<player<<"\","<<operation<<","<<"\""<<pass<<"\")"<<std::endl;
    if(pass!=this->pass)
    {
        std::cout<<"[Reply]error:"<<strResult(AUTH_ERROR)<<std::endl;
        return_error((int)AUTH_ERROR);
        return;
    }
    auto err = this->serverMgr->whitelist_op(player,operation);
    if(err)
    {
        std::cout<<"[Reply]error:"<<strResult((Result)err)<<std::endl;
        return_error((int)err);
    }
    else
    {
        std::cout<<"[Reply]"<<"SUCCESS"<<std::endl;
        return_result("SUCCESS");
    }
    return;
}

void WebApiMain::op(std::string player, bool operation, std::string pass) {

    std::cout<<"[HTTP Call]op(\""<<player<<"\","<<operation<<",\""<<pass<<"\")"<<std::endl;
    if(pass!=this->pass)
    {
        std::cout<<"[Reply]error:"<<strResult(AUTH_ERROR)<<std::endl;
        return_error((int)AUTH_ERROR);
        return;
    }
    int err = this->serverMgr->op(operation,player);
    if(err)
    {
        std::cout<<"[Reply]error:"<<strResult((Result)err)<<std::endl;
        return_error(err);
    }
    else
    {
        std::cout<<"[Reply]"<<"SUCCESS"<<std::endl;
        return_result("SUCCESS");
    }
}

void WebApiMain::kick(std::string player,std::string reason, std::string pass) {
    std::cout<<"[HTTP Call]kick(\""<<player<<"\",\""<<reason<<"\",\""<<pass<<"\")"<<std::endl;
    if(pass!=this->pass) {
        std::cout << "[Reply]error:" << strResult(AUTH_ERROR) << std::endl;
        return_error((int) AUTH_ERROR);
        return;
    }
    auto err=this->serverMgr->kick(player,reason);
    if(err)
    {
        std::cout<<"[Reply]error:"<<strResult((Result)err)<<std::endl;
        return_error(err);
    }
    else
    {
        std::cout<<"[Reply]"<<"SUCCESS"<<std::endl;
        return_result("SUCCESS");
    }
}

void WebApiMain::say(std::string words, std::string pass)
{
    std::cout<<"[HTTP Call]say(\""<<words<<"\",\""<<pass<<"\")"<<std::endl;
    if(pass!=this->pass) {
        std::cout << "[Reply]error:" << strResult(AUTH_ERROR) << std::endl;
        return_error((int) AUTH_ERROR);
        return;
    }
    auto err=this->serverMgr->say(words);
    if(err)
    {
        std::cout<<"[Reply]error:"<<strResult((Result)err)<<std::endl;
        return_error(err);
    }
    else
    {
        std::cout<<"[Reply]"<<"SUCCESS"<<std::endl;
        return_result("SUCCESS");
    }
}

void WebApiMain::kill(std::string name,std::string pass)
{
    std::cout<<"[HTTP Call]kill(\""<<name<<"\",\""<<pass<<"\")"<<std::endl;
    if(pass!=this->pass) {
        std::cout << "[Reply]error:" << strResult(AUTH_ERROR) << std::endl;
        return_error((int) AUTH_ERROR);
        return;
    }
    auto err = this->serverMgr->kill(name);
    if(err)
    {
        std::cout<<"[Reply]error:"<<strResult((Result)err)<<std::endl;
        return_error((int)err);
    }
    else
    {
        std::cout<<"[Reply]"<<"SUCCESS"<<std::endl;
        return_result("SUCCESS");
    }

}

void WebApiMain::gamemode(std::string name, int mode, std::string pass)
{
    std::cout<<"[HTTP Call]gamemode(\""<<name<<"\","<<mode<<",\""<<pass<<"\")"<<std::endl;
    if(pass!=this->pass) {
        std::cout << "[Reply]error:" << strResult(AUTH_ERROR) << std::endl;
        return_error((int) AUTH_ERROR);
        return;
    }
    auto err = this->serverMgr->gamemode(name,mode);
    if(err)
    {
        std::cout<<"[Reply]error:"<<strResult((Result)err)<<std::endl;
        return_error((int)err);
    }
    else
    {
        std::cout<<"[Reply]"<<"SUCCESS"<<std::endl;
        return_result("SUCCESS");
    }

}

void WebApiMain::permission_set(std::string name, int mode, std::string pass)
{
    std::cout<<"[HTTP Call]permission_set(\""<<name<<"\","<<mode<<",\""<<pass<<"\")"<<std::endl;
    if(pass!=this->pass) {
        std::cout << "[Reply]error:" << strResult(AUTH_ERROR) << std::endl;
        return_error((int) AUTH_ERROR);
        return;
    }
    auto err = this->serverMgr->permission_set(name,mode);
    if(err)
    {
        std::cout<<"[Reply]error:"<<strResult((Result)err)<<std::endl;
        return_error((int)err);
    }
    else
    {
        std::cout<<"[Reply]"<<"SUCCESS"<<std::endl;
        return_result("SUCCESS");
    }
}
