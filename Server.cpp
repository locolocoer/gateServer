#include "Server.h"
#include "HttpConnection.h"
#include "IOContextPool.h"
Server::Server(boost::asio::io_context& ioc, unsigned short port):_ioc(ioc),_port(port),_acceptor(ioc,tcp::endpoint(asio::ip::address_v4::any(),port))
{
}

void Server::start()
{
	auto self = shared_from_this();
	auto& ioc = IOContextPool::getInstance()->getIOContext();
	std::shared_ptr<HttpConnection> new_con = std::make_shared<HttpConnection>(ioc);
	_acceptor.async_accept(new_con->get_socket(), [self,new_con](boost::system::error_code ec) {
			if (ec) {
				std::cout << "Error code: " <<ec.value()<< ec.message() << std::endl;
				return;
			}
			std::cout << "Connection accepted from " << new_con->get_socket().remote_endpoint() << std::endl;
			new_con->start();
			self->start();

		});
}


void Server::start_accept() {
	auto self = shared_from_this();
	std::shared_ptr<HttpConnection> new_con = std::make_shared<HttpConnection>(_ioc);
	_acceptor.async_accept(new_con->get_socket(),
		[this,new_con](const boost::system::error_code& error) {
			if (!error) {
				std::cout << "Connection accepted from " << new_con->get_socket().remote_endpoint() << std::endl;
				// 处理新连接，例如启动读写操作
			}
			else {
				std::cerr << "Accept error: " << error.message() << std::endl;
			}
			// 无论成功还是失败，都再次启动异步接受
			start_accept();
		});
}