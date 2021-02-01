//
// Created by danny on 2021/1/31.
//

#ifndef BEDROCKMGR_LOGGER_H
#define BEDROCKMGR_LOGGER_H

#include<string>
#include<unistd.h>
#include"../util/SystemException.h"
class Logger
{
public:
    explicit Logger(std::string filePath);
    ~Logger();
    Logger& operator<<(const std::string& what);

private:
    int32_t fd;

};


#endif //BEDROCKMGR_LOGGER_H
