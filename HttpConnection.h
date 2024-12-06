#pragma once
#include "const.h"
class HttpConnection:public std::enable_shared_from_this<HttpConnection>
{
public:
	HttpConnection(asio::io_context& ioc);
	tcp::socket& get_socket();
	void start();
	void handleReq();
	void writeResponse();
	void checkDeadline();
	http::response<http::dynamic_body>& get_response();
	http::request<http::dynamic_body>& get_request();
	void parseGetParams();
	std::map<std::string, std::string> get_get_params();
private:
	tcp::socket _socket;
	beast::flat_buffer _buffer;
	http::request<http::dynamic_body> _request;
	http::response<http::dynamic_body> _response;
	asio::steady_timer _timer{ _socket.get_executor(),std::chrono::seconds(60) };
		std::string _get_url;
	std::map<std::string, std::string> _get_params;
};

