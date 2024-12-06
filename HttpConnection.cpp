#include "HttpConnection.h"
#include "logicSystem.h"
HttpConnection::HttpConnection(asio::io_context& ioc):_socket(ioc)
{
}

tcp::socket& HttpConnection::get_socket()
{
    return _socket;
}

void HttpConnection::start()
{
    auto self = shared_from_this();
    http::async_read(_socket, _buffer, _request,[self](boost::beast::error_code ec, std::size_t bytes_transfered){
        if (ec) {
            std::cout << "Error message: " << ec.message() << std::endl;
            return;
            }
        self->handleReq();
        self->checkDeadline();
        });
}

void HttpConnection::handleReq()
{
    auto self = shared_from_this();
    _response.version(_request.version());
    _response.keep_alive(false);
    if (_request.method() == http::verb::get) {
		parseGetParams();
        bool success = logicSystem::getInstance()->handlGet(_get_url,self);
        if (!success) {
            _response.result(http::status::not_found);
            _response.set(http::field::content_type, "text/plain");
            beast::ostream(_response.body()) << "url not found\r\n";
            writeResponse();
            return;
        }
        _response.result(http::status::ok);
        _response.set(http::field::content_type, "text/plain");
        writeResponse();
    }
    if (_request.method() == http::verb::post) {
        bool sucess = logicSystem::getInstance()->handlPost(_request.target(), self);
        if (!sucess) {
            _response.result(http::status::not_found);
            beast::ostream(_response.body()) << "url not found\r\n";
            writeResponse();
            return;
        }
        _response.result(http::status::ok);
        writeResponse();
    }
}

void HttpConnection::writeResponse()
{
    auto self = shared_from_this();
    _response.content_length(_response.body().size());
    http::async_write(_socket, _response, [self](boost::beast::error_code ec, std::size_t byte_transfered) {
        if (ec) {
            std::cout << "Error message: " << ec.message() << std::endl;
            }
        self->get_socket().shutdown(tcp::socket::shutdown_send, ec);
        self->_timer.cancel();
        });
}

void HttpConnection::checkDeadline()
{
    auto self = shared_from_this();
    _timer.async_wait([self](boost::system::error_code ec) {
        if (!ec) {
            self->get_socket().close();
        }
        });
}

http::response<http::dynamic_body>& HttpConnection::get_response()
{
    
    return _response;
}

http::request<http::dynamic_body>& HttpConnection::get_request()
{
    return _request;
}



char toHex(unsigned char x) {
    if (x > 9) {
        return x + 55;
    }
    else {
        return x + 48;
    }
}

unsigned char fromHex(char x) {
    if (x >= 'A' && x <= 'Z') {
        return x - 'A' + 10;
    }
    else if (x >= 'a' && x <= 'z') {
        return x - 'a' + 10;
    }
    else if (x >= '0' && x <= '9') {
        return x - '0';
    }
    return -1;
}

std::string urlEncode(std::string url) {
    std::string ans;
    int n = url.length();
    for (int i = 0; i < n; i++) {
        if (std::isalnum(url[i]) ||
            (url[i] == '_') ||
            (url[i] == '-') ||
            (url[i] == '.') ||
            (url[i] == '~')) {
            ans += url[i];
        }
        else if (url[i] == ' ') {
            ans += '+';
        }
        else {
            ans += '%';
            ans += toHex((unsigned char)url[i] >> 4);
            ans += toHex((unsigned char)url[i] & 0x0F);
        }
    }
    return ans;
}

std::string urlDecode(std::string url) {
    std::string ans;
    int n = url.length();
    for (int i = 0; i < n; i++) {
        if (url[i] == '%') {
            char high = fromHex(url[++i]);
            char low = fromHex(url[++i]);
            ans += 16 * high + low;
        }
        else if (url[i] == '+') {
            ans += ' ';
        }
        else {
            ans += url[i];
        }
    }
    return ans;
}


void HttpConnection::parseGetParams()
{
	std::string uri = _request.target();
	if (uri.find('?') == std::string::npos) {
		_get_url = uri;
		return;
	}
	
	auto query_pos = uri.find('?');
	_get_url = uri.substr(0, query_pos);
	auto query = uri.substr(query_pos+1);
	std::size_t pos = 0;
	std::string key;
	std::string value;
	while ((pos = query.find('&')) != std::string::npos) {
		auto pair = query.substr(0, pos);
		auto eq_pos = pair.find('=');
		if (eq_pos != std::string::npos) {
			key = urlDecode(pair.substr(0, eq_pos));
			value = urlDecode(pair.substr(eq_pos + 1));
			_get_params[key] = value;
		}
		query.erase(0, pos + 1);
	}
	if (!query.empty()) {
		auto pair = query.substr(0, pos);
		auto eq_pos = pair.find('=');
		if (eq_pos != std::string::npos) {
			key = urlDecode(pair.substr(0, eq_pos));
			value = urlDecode(pair.substr(eq_pos + 1));
			_get_params[key] = value;
		}
	}
}

std::map<std::string, std::string> HttpConnection::get_get_params()
{
	return _get_params;
}

