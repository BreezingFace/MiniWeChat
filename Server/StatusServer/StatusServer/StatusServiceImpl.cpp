#include "StatusServiceImpl.h"
#include "ConfigMgr.h"
#include "const.h"
#include "RedisMgr.h"
#include <climits>
#include<boost/uuid/uuid_generators.hpp>
#include<boost/uuid/uuid_io.hpp>

std::string generate_unique_string() {
	// 创建UUID对象
	boost::uuids::uuid uuid = boost::uuids::random_generator()();

	// 将UUID转换为字符串
	std::string unique_string = to_string(uuid);

	return unique_string;
}
StatusServiceImpl::StatusServiceImpl() 
{
	auto& cfg = ConfigMgr::Inst();
	auto server_list = cfg["chatservers"]["Name"];

	std::vector<std::string> words;

	std::stringstream ss(server_list);
	std::string word;

	while (std::getline(ss, word, ',')) {
		words.push_back(word);
	}

	for (auto& word : words) {
		if (cfg[word]["Name"].empty()) {
			continue;
		}

		ChatServer server;
		server.port = cfg[word]["Port"];
		server.host = cfg[word]["Host"];
		server.name = cfg[word]["Name"];
		_servers[server.name] = server;
	}
}
//Status StatusServiceImpl::GetChatServer(ServerContext* context, const GetChatServerReq* request, GetChatServerRsp* reply)
//{
//	std::string prefix("crh status server has received :  ");
//	_server_index = (_server_index++) % (_servers.size());
//	auto& server = _servers[_server_index];
//	reply->set_host(server.host);
//	reply->set_port(server.port);
//	reply->set_error(ErrorCodes::Success);
//	reply->set_token(generate_unique_string());
//	return Status::OK;
//}
Status StatusServiceImpl::GetChatServer(ServerContext* context, const GetChatServerReq* request, GetChatServerRsp* reply)
{
	std::string prefix("crh status server has received :  ");
	const auto& server = getChatServer();//获取负载最小的ChatServer
	reply->set_host(server.host);
	reply->set_port(server.port);
	reply->set_error(ErrorCodes::Success);
	reply->set_token(generate_unique_string());
	insertToken(request->uid(), reply->token());
	return Status::OK;
}

//StatusServiceImpl::StatusServiceImpl()
//{
//	auto& cfg = ConfigMgr::Inst();
//	auto server_list = cfg["chatservers"]["Name"];
//
//	std::vector<std::string> words;
//
//	std::stringstream ss(server_list);
//	std::string word;
//
//	while (std::getline(ss, word, ',')) {
//		words.push_back(word);
//	}
//
//	for (auto& word : words) {
//		if (cfg[word]["Name"].empty()) {
//			continue;
//		}
//
//		ChatServer server;
//		server.port = cfg[word]["Port"];
//		server.host = cfg[word]["Host"];
//		server.name = cfg[word]["Name"];
//		_servers[server.name] = server;
//	}
//
//}

ChatServer StatusServiceImpl::getChatServer() {
	std::lock_guard<std::mutex> guard(_server_mtx);
	auto minServer = _servers.begin()->second;
	auto count_str = RedisMgr::GetInstance()->HGet(LOGIN_COUNT, minServer.name);
	if (count_str.empty()) {
		//不存在则默认设置为最大
		minServer.con_count = INT_MAX;
	}
	else {
		minServer.con_count = std::stoi(count_str);
	}


	// 使用范围基于for循环
	for (auto& server : _servers) {

		if (server.second.name == minServer.name) {
			continue;
		}

		auto count_str = RedisMgr::GetInstance()->HGet(LOGIN_COUNT, server.second.name);//获取server.second.name的连接数
		if (count_str.empty()) {
			server.second.con_count = INT_MAX;
		}
		else {
			server.second.con_count = std::stoi(count_str);
		}

		if (server.second.con_count < minServer.con_count) {
			minServer = server.second;
		}
	}

	return minServer;
}

Status StatusServiceImpl::Login(ServerContext* context, const LoginReq* request, LoginRsp* reply)
{
	auto uid = request->uid();
	auto token = request->token();

	std::string uid_str = std::to_string(uid);
	std::string token_key = USERTOKENPREFIX + uid_str;
	std::string token_value = "";
	bool success = RedisMgr::GetInstance()->Get(token_key, token_value);
	if (!success) {
		reply->set_error(ErrorCodes::UidInvalid);
		return Status::OK;
	}
	//在 gRPC 服务端的 Login 方法中验证 Token 是否与 Redis 中存储的 Token 匹配，主要是为了确保用户身份的合法性和安全性。
	if (token_value != token) {
		reply->set_error(ErrorCodes::TokenInvalid);
		return Status::OK;
	}
	//如果用户给的token和redis中的token匹配，则将Status::OK返回给chatserver
	reply->set_error(ErrorCodes::Success);
	reply->set_uid(uid);
	reply->set_token(token);
	return Status::OK;
}

void StatusServiceImpl::insertToken(int uid, std::string token)
{   
//	std::lock_guard<std::mutex> guard(_token_mtx);
	std::string uid_str = std::to_string(uid);
	std::string token_key = USERTOKENPREFIX + uid_str;
	RedisMgr::GetInstance()->Set(token_key, token);
}

