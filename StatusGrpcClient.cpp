#include "StatusGrpcClient.h"
#include "ConfigMgr.h"
#include "const.h"
GetChatRsp StatusGrpcClient::getChatServer(int uid)
{
	ClientContext context;
	GetChatRsp rsp;
	GetChatReq req;
	req.set_uid(uid);
	Status status = _stub->GetChatServer(&context, req,&rsp);
	if (status.ok()) {
		return rsp;
	}
	rsp.set_error(ErrorCodes::ErrorGrpc);
	return rsp;

}
StatusGrpcClient::StatusGrpcClient()
{
	auto cfgMgr = ConfigMgr::Inst();
	auto host = cfgMgr["StatusServer"]["Host"];
	auto port = cfgMgr["StatusServer"]["Port"];
	auto channel = grpc::CreateChannel(host + ":" + port, grpc::InsecureChannelCredentials());
	_stub = StatusService::NewStub(channel);
}
