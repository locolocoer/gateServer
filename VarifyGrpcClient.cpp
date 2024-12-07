#include "VarifyGrpcClient.h"
#include "ConfigMgr.h"
GetVarifyRsp VarifyGrpcClient::GetVarifyCode(std::string email)
{
	ClientContext context;
	GetVarifyReq req;
	GetVarifyRsp rsp;
	req.set_email(email);
	auto status = _stub->GetVarifyCode(&context, req, &rsp);
	if (status.ok()) {
		return  rsp;
	}
	rsp.set_code(email.c_str());
	return rsp;
}

VarifyGrpcClient::VarifyGrpcClient()
{
	auto cfgMgr = ConfigMgr::Inst();
	auto host = cfgMgr["VarifyServer"]["Host"];
	auto port = cfgMgr["VarifyServer"]["Port"];
	std::shared_ptr<Channel> channle = grpc::CreateChannel(host+":"+port, grpc::InsecureChannelCredentials());
	_stub = VarifyService::NewStub(channle);
}
