// myGateServer.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <boost/asio.hpp>
#include "Server.h"
#include "const.h"
#include "MysqlDao.h"
int main()
{
    auto inst = MysqlDao::getInstance();
    
    try {
        boost::asio::io_context ioc;
        boost::asio::signal_set signal(ioc, SIGINT, SIGTERM);
        signal.async_wait([&](boost::system::error_code ec,int signal_num) {
            if (ec) {
                return;
            }
            ioc.stop();
            });
        unsigned short port = 10086;
        std::shared_ptr<Server> s = std::make_shared<Server>(ioc, port);
        s->start();
        ioc.run();
    }
    catch (std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
    }
}


