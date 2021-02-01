//
// Created by danny on 2021/1/20.
//

#include "SystemException.h"
#include<cstring>
#include<errno.h>
#include<unistd.h>
SystemException::SystemException(int errorno)
{
    strerror_r(errorno,this->info,SYS_EXCEPT_CHAR_LEN);
}
SystemException::SystemException()
{
    strerror_r(errno,this->info,SYS_EXCEPT_CHAR_LEN);
}