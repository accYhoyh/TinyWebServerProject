cmake_minimum_required(VERSION 2.8)
project(TinyWebServer)

# 【核心回答】：添加头文件搜索路径
# 这样你可以直接 include 各个模块的头文件，如 #include "locker.h"
# 或者添加 ${PROJECT_SOURCE_DIR} 后写 #include "lock/locker.h"
include_directories(
    ${PROJECT_SOURCE_DIR}/lock
    ${PROJECT_SOURCE_DIR}/CGImysql
    ${PROJECT_SOURCE_DIR}/threadpool
    ${PROJECT_SOURCE_DIR}/http
    ${PROJECT_SOURCE_DIR}/timer
    ${PROJECT_SOURCE_DIR}/log
)

# 查找 MySQL 库 (对应 makefile 中的 -lmysqlclient)
# 如果系统路径找不到，可能需要手动指定 link_directories
find_library(MYSQL_LIB mysqlclient)


# 定义源文件列表
set(SOURCE_FILES
    main.cpp
    webserver.cpp
    ${PROJECT_SOURCE_DIR}/CGImysql/sql_connection_pool.cpp
    ${PROJECT_SOURCE_DIR}/http/http_conn.cpp
    ${PROJECT_SOURCE_DIR}/timer/lst_timer.cpp
    ${PROJECT_SOURCE_DIR}/log/log.cpp
)

# 生成可执行文件
add_executable(server ${SOURCE_FILES})

# 链接库 (对应 -lpthread -lmysqlclient)
target_link_libraries(server pthread ${MYSQL_LIB})