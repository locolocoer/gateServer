#pragma once
#include <grpcpp/grpcpp.h>
#include "Singleton.h"
#include "msg.grpc.pb.h"

using grpc::Channel;
using message::StatusService;
class StatusConPool 
{
public:
	std::unique_ptr<StatusService::Stub> getConnection();
	void returnConection(std::unique_ptr<StatusService::Stub>);
	void close();
	~StatusConPool();
	StatusConPool(std::string host, std::string port, int poolsize);
private:
	std::queue<std::unique_ptr<StatusService::Stub>> _pool;
	std::mutex _mutex;
	std::condition_variable cv;
	std::atomic_bool _b_stop;
	
};

