#pragma once
#include <grpcpp/grpcpp.h>
#include "msg.grpc.pb.h"
#include "Singleton.h"
#include "StatusConPool.h"
using grpc::Channel;
using grpc::Status;
using grpc::ClientContext;
using message::GetChatReq;
using message::GetChatRsp;
using message::StatusService;
class StatusGrpcClient:public Singleton<StatusGrpcClient>
{
	friend class Singleton<StatusGrpcClient>;
public:
	GetChatRsp getChatServer(int uid);
private:
	StatusGrpcClient();
	std::unique_ptr<StatusConPool> _pool;
};

