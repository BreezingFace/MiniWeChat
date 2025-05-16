#pragma once
#include <grpcpp/grpcpp.h>
#include "message.grpc.pb.h"
#include <mutex>

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using message::GetChatServerReq;
using message::GetChatServerRsp;
using message::LoginReq;
using message::LoginRsp;
using message::StatusService;
/*
* 
* 我有2个Server，一个是StatusServer，一个是ChatServer，它们分别是grpc的服务端和客户端，我现在的疑惑是，它们共用同一个proto文件，
为什么呈现出服务端和客户端的区别，以及它们是如何进行通讯的呢？
为什么共用 proto 文件却能区分服务端和客户端？原因在于proto 文件的角色
接口契约：proto 文件定义了服务接口和消息格式，是双方通信的"协议"
双向定义：同一个 proto 会生成：
服务端骨架代码（需要实现的具体服务）
客户端存根代码（用于调用远程服务的代理）
特性	     服务端 (StatusServer)	                 客户端 (ChatServer)
代码角色	实现proto定义的服务方法	                 调用proto定义的服务方法
网络角色	监听端口，等待连接	                     主动建立连接到服务端
线程模型	通常多线程处理并发请求	                 通常单线程发起调用
典型代码	继承 XxxService::Service 并实现方法	   通过 XxxService::Stub 调用远程方法
生命周期	长期运行	                             按需创建/销毁
*/
class  ChatServer {
public:
	ChatServer():host(""),port(""),name(""),con_count(0){}
	ChatServer(const ChatServer& cs):host(cs.host), port(cs.port), name(cs.name), con_count(cs.con_count){}
	ChatServer& operator=(const ChatServer& cs) {
		if (&cs == this) {
			return *this;
		}

		host = cs.host;
		name = cs.name;
		port = cs.port;
		con_count = cs.con_count;
		return *this;
	}
	std::string host;
	std::string port;
	std::string name;
	int con_count;
};
//struct ChatServer {
//	std::string host;
//	std::string port;
//	int con_count;
//};

class StatusServiceImpl final : public StatusService::Service
{
public:
	StatusServiceImpl();
	Status GetChatServer(ServerContext* context, const GetChatServerReq* request,
		GetChatServerRsp* reply) override;
	
	//std::vector<ChatServer> _servers;
	//int _server_index;
   	Status Login(ServerContext* context, const LoginReq* request,
		LoginRsp* reply) override;
private:
	void insertToken(int uid, std::string token);
	ChatServer getChatServer();
	std::unordered_map<std::string, ChatServer> _servers;
	std::mutex _server_mtx;
	//std::unordered_map<int, std::string> _tokens;
	//std::mutex _token_mtx;

};

