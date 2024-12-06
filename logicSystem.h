#pragma once
#include "Singleton.h"
#include "const.h"
#include "HttpConnection.h"
typedef std::function<void(std::shared_ptr<HttpConnection>)> httpHandler;
class logicSystem:public Singleton<logicSystem>
{
	friend class Singleton<logicSystem>;
private:
	logicSystem();
	std::map<std::string, httpHandler> get_handlers;
	std::map<std::string, httpHandler> post_handlers;

public:
	void regGet();
	bool handlGet(std::string paht,std::shared_ptr<HttpConnection> con);
	void regPost();
	bool handlPost(std::string path, std::shared_ptr<HttpConnection> con);
};

