#include "logicSystem.h"
#include "VarifyGrpcClient.h"
#include "RedisMgr.h"
#include "MysqlMgr.h"
#include "StatusGrpcClient.h"
logicSystem::logicSystem()
{
	regGet();
	regPost();
}

void logicSystem::regGet()
{
	get_handlers["/get_test"] = [](std::shared_ptr<HttpConnection> con) {
		beast::ostream(con->get_response().body()) << "get_test success" << std::endl;
		for (auto pair : con->get_get_params()) {
			std::cout << "key:" << pair.first << "," << " value:" << pair.second << std::endl;
		}
		};
}

bool logicSystem::handlGet(std::string path, std::shared_ptr<HttpConnection> con)
{
	std::cout << path << std::endl;
	if (get_handlers.find(path) == get_handlers.end()) {
		return false;
	}
	get_handlers[path](con);
	return true;
}

void logicSystem::regPost()
{
	post_handlers["/get_varify"] = [](std::shared_ptr<HttpConnection> con) {
		
		auto data_str = beast::buffers_to_string(con->get_request().body().data());
		con->get_response().set(http::field::content_type, "text/json");
		Json::Value src_root, root;
		Json::Reader parse;
		auto isSucss = parse.parse(data_str, src_root);
		if (!isSucss) {
			std::cout << "Parse json failed!" << std::endl;
			root["Error"] = ErrorCodes::ErrorJson;
			beast::ostream(con->get_response().body()) << root.toStyledString();
		}
		root["Error"] = ErrorCodes::Sucess;
		std::string email = src_root["Email"].asString();
		GetVarifyRsp rsp = VarifyGrpcClient::getInstance()->GetVarifyCode(email);
		if (rsp.error()) {
			root["Error"] = ErrorCodes::ErrorGrpc;
			beast::ostream(con->get_response().body()) << root.toStyledString();
		}
		root["Email"] = src_root["Email"];
		root["code"] = rsp.code();
		beast::ostream(con->get_response().body()) << root.toStyledString();
		};
	post_handlers["/register"] = [](std::shared_ptr<HttpConnection> con) {
		auto data_str = beast::buffers_to_string(con->get_request().body().data());
		con->get_response().set(http::field::content_type, "text/json");
		Json::Value src_root, root;
		Json::Reader parser;
		bool isSucss = parser.parse(data_str, src_root);
		if (!isSucss) {
			std::cout << "Parse json failed!" << std::endl;
			root["Error"] = ErrorCodes::ErrorJson;
			beast::ostream(con->get_response().body()) << root.toStyledString();
			return false;
		}
		auto redismgr = RedisMgr::getInstance();
		auto email = src_root["Email"].asString();
		std::string varify_code;
		bool isGetSucss = redismgr->Get(email, varify_code);
		if (!isGetSucss) {
			std::cout << "Varify code expired!" << std::endl;
			root["Error"] = ErrorCodes::VarifyExpired;
			beast::ostream(con->get_request().body()) << root.toStyledString();
			return false;
		}
		if (varify_code != src_root["varifyCode"].asString()) {
			std::cout << "Varify code not match" << std::endl;
			root["Error"] = ErrorCodes::VarifyError;
			beast::ostream(con->get_request().body()) << root.toStyledString();
			return false;
		}

		auto sqlMgr = MysqlMgr::getInstance();
		auto userid = sqlMgr->regUser(root["User"].asString(), root["Email"].asString(), root["Password"].asString());
		if (userid == 0 || userid == -1) {
			std::cout << "user already exist" << std::endl;
			root["Error"] = ErrorCodes::UserAlreadyExist;
			root["User"] = src_root["User"];
			root["Emial"] = src_root["Email"];
			beast::ostream(con->get_response().body()) << root.toStyledString();
			return false;
		}
		root["Error"] = ErrorCodes::Sucess;
		root["User"] = src_root["User"];
		root["Uid"] = userid;
		beast::ostream(con->get_request().body()) << root.toStyledString();
		};
	post_handlers["/reset_password"] = [](std::shared_ptr<HttpConnection> con) {
		auto datastr = boost::beast::buffers_to_string(con->get_request().body().data());
		Json::Value root, src_root;
		Json::Reader reader;
		bool isSucess = reader.parse(datastr, src_root);
		if (!isSucess) {
			std::cout << "Parse json failed!" << std::endl;
			root["Error"] = ErrorCodes::ErrorJson;
			beast::ostream(con->get_response().body()) << root.toStyledString();
			return false;
		}
		auto email = src_root["Email"].asString();
		auto username = src_root["User"].asString();
		auto pwd = src_root["Password"].asString();
		std::string varifyCode;
		auto res = RedisMgr::getInstance()->Get(email, varifyCode);
		if (!res) {
			std::cout << "Varify code expired!" << std::endl;
			root["Error"] = ErrorCodes::VarifyExpired;
			beast::ostream(con->get_request().body()) << root.toStyledString();
			return false;
		}
		if (varifyCode != src_root["varifyCode"].asString()) {
			std::cout << "Varify code not match" << std::endl;
			root["Error"] = ErrorCodes::VarifyError;
			beast::ostream(con->get_request().body()) << root.toStyledString();
			return false;
		}
		bool usrMatchEmail = MysqlMgr::getInstance()->checkUsrMatchEmail(src_root["User"].asString(), src_root["Email"].asString());
		if (!usrMatchEmail) {
			std::cout << "Username and Email are not matched" << std::endl;
			root["Error"] = ErrorCodes::usrNotMatchEmail;
			beast::ostream(con->get_request().body()) << root.toStyledString();
			return false;
		}
		bool updateSucess = MysqlMgr::getInstance()->updatePwd(src_root["Email"].asString(), src_root["Password"].asString());
		if (!updateSucess) {
			std::cout << "update password failed" << std::endl;
			root["Error"] = ErrorCodes::upPwdFailed;
			beast::ostream(con->get_request().body()) << root.toStyledString();
			return false;
		}
		root["Error"] = ErrorCodes::Sucess;
		beast::ostream(con->get_request().body()) << root.toStyledString();
		return true;
		};
	post_handlers["/login"] = [](std::shared_ptr<HttpConnection> con) {
		auto datastr = boost::beast::buffers_to_string(con->get_request().body().data());
		Json::Value root, src_root;
		Json::Reader reader;
		bool isSucess = reader.parse(datastr, src_root);
		if (!isSucess) {
			std::cout << "Parse json failed!" << std::endl;
			root["Error"] = ErrorCodes::ErrorJson;
			beast::ostream(con->get_response().body()) << root.toStyledString();
			return false;
		}
		auto email = src_root["Email"].asString();
		auto username = src_root["User"].asString();
		auto pwd = src_root["Password"].asString();
		UserInfo info;
		bool isMatch = MysqlMgr::getInstance()->checkPwd(username, pwd, info);
		if (!isMatch) {
			std::cout << "pwd not correct!" << std::endl;
			root["Error"] = ErrorCodes::PwdNameNotMatch;
			beast::ostream(con->get_response().body()) << root.toStyledString();
			return false;
		}
		auto res = StatusGrpcClient::getInstance()->getChatServer(info.uid);
		if (res.error()) {
			std::cout << "GRPC FAILED!" << std::endl;
			root["Error"] = ErrorCodes::ErrorGrpc;
			beast::ostream(con->get_response().body()) << root.toStyledString();
			return false;
		}
		root["Error"] = ErrorCodes::Sucess;
		root["User"] = username;
		root["Email"] = email;
		root["Password"] = pwd;
		root["Uid"] = info.uid;
		root["Token"] = res.token();
		root["Host"] = res.host();
		beast::ostream(con->get_response().body()) << root.toStyledString();
		return true;
		};
}

bool logicSystem::handlPost(std::string path, std::shared_ptr<HttpConnection> con)
{
	std::cout << path << std::endl;
	if (post_handlers.find(path) == post_handlers.end()) {
		return false;
	}
	post_handlers[path](con);
	return true;
}



