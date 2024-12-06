#pragma once
#include "const.h"
class Server:public std::enable_shared_from_this<Server>
{
public:
	Server(boost::asio::io_context& ioc, unsigned short port);
	void start();
	void start_accept();
private:
	boost::asio::io_context& _ioc;
	unsigned short _port;
	boost::asio::ip::tcp::acceptor _acceptor;
};

