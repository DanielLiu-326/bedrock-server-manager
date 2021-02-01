//
// Created by danny on 2021/1/20.
//

#ifndef BEDROCKMGR_SYSTEMEXCEPTION_H
#define BEDROCKMGR_SYSTEMEXCEPTION_H
#include<exception>
#define SYS_EXCEPT_CHAR_LEN 256
class SystemException :public std::exception
{
public:
    char info[SYS_EXCEPT_CHAR_LEN];
    SystemException(int errorno);
    SystemException();
    inline const char * what() const noexcept
    {
        return this->info;
    }
};


#endif //BEDROCKMGR_SYSTEMEXCEPTION_H
