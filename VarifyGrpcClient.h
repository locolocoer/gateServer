#pragma once
#include <grpcpp/grpcpp.h>
#include "msg.grpc.pb.h"
#include "Singleton.h"
using grpc::Channel;
using grpc::Status;
using grpc::ClientContext;
using message::GetVarifyReq;
using message::GetVarifyRsp;
using message::VarifyService;
class VarifyGrpcClient:public Singleton<VarifyGrpcClient>
{
	friend class Singleton<VarifyGrpcClient>;
public:
	GetVarifyRsp GetVarifyCode(std::string email);
private:
	VarifyGrpcClient();
	std::unique_ptr<VarifyService::Stub> _stub;
};

