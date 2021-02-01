PROJECT(BedrockMgr)
CMAKE_MINIMUM_REQUIRED(VERSION 2.4)
set(SOURCES
        src/Logger/Logger.cpp
        src/Logger/Logger.h
        src/AppRunner/AppRunner.cpp
        src/AppRunner/AppRunner.h
        src/util/md5.h
        src/util/md5.cpp
        src/util/SystemException.h
        src/util/SystemException.cpp
        src/util/WrappedFunc.cpp
        src/util/WrappedFunc.h
        src/ServerMgr.cpp
        src/ServerMgr.h
        src/main.cpp
        src/WebApiMain.h
        src/WebApiMain.cpp
        )

add_executable(BedrockMgr ${SOURCES})
target_link_libraries(BedrockMgr pthread)
target_link_libraries(BedrockMgr cppcms)