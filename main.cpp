#include "webserver.h"

int main(int argc, char *argv[])
{
    // 1. 数据库配置
    string user = "hoyh";
    string passwd = "123456";
    string databasename = "tinywebserver";

    // 2. 【核心修改】直接定义配置变量，不使用 Config 类
    int port = 9190;          // 端口
    int log_write = 0;        // 0:同步日志, 1:异步日志
    int trig_mode = 3;        // 0:LT+LT, 1:LT+ET, 2:ET+LT, 3:ET+ET
    int opt_linger = 1;       // 0:不优雅关闭, 1:优雅关闭
    int sql_num = 8;          // 数据库连接池数量
    int thread_num = 10;       // 线程池数量
    int close_log = 1;        // 0:开启日志, 1:关闭日志
    int actor_model = 0;      // 0:Proactor, 1:Reactor

    // 3. 创建服务器对象
    WebServer server;

    // 4. 初始化 (直接传入上面定义的变量)
    server.init(port, user, passwd, databasename, log_write, 
                opt_linger, trig_mode, sql_num, thread_num, 
                close_log, actor_model);

    // 5. 后续启动流程 (保持不变)
    server.log_write();
    server.sql_pool();
    server.thread_pool();
    server.trig_mode();
    server.eventListen();
    server.eventLoop();

    return 0;
}