#include "VarifyGrpcClient.h"

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
	std::shared_ptr<Channel> channle = grpc::CreateChannel("127.0.0.1:10001", grpc::InsecureChannelCredentials());
	_stub = VarifyService::NewStub(channle);
}
