//
// Created by danny on 2021/1/20.
//

#ifndef BEDROCKMGR_WRAPEDFUNC_H
#define BEDROCKMGR_WRAPEDFUNC_H
#include<string>
#include<vector>
extern int sigBlock(int signo);
extern int sigUnblock(int signo);
extern std::vector<std::string> getEnviron();
extern int setNonblock(int fd);
extern int clearPipe(int readFd);
extern std::vector<std::string> splitStr(const std::string& str,char split);
extern int copyFile(std::string from ,std::string to);
extern std::string trimStr(const std::string& str);
#endif //BEDROCKMGR_WRAPEDFUNC_H
