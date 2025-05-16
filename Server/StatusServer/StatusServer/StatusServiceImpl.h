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
* ����2��Server��һ����StatusServer��һ����ChatServer�����Ƿֱ���grpc�ķ���˺Ϳͻ��ˣ������ڵ��ɻ��ǣ����ǹ���ͬһ��proto�ļ���
Ϊʲô���ֳ�����˺Ϳͻ��˵������Լ���������ν���ͨѶ���أ�
Ϊʲô���� proto �ļ�ȴ�����ַ���˺Ϳͻ��ˣ�ԭ������proto �ļ��Ľ�ɫ
�ӿ���Լ��proto �ļ������˷���ӿں���Ϣ��ʽ����˫��ͨ�ŵ�"Э��"
˫���壺ͬһ�� proto �����ɣ�
����˹Ǽܴ��루��Ҫʵ�ֵľ������
�ͻ��˴�����루���ڵ���Զ�̷���Ĵ���
����	     ����� (StatusServer)	                 �ͻ��� (ChatServer)
�����ɫ	ʵ��proto����ķ��񷽷�	                 ����proto����ķ��񷽷�
�����ɫ	�����˿ڣ��ȴ�����	                     �����������ӵ������
�߳�ģ��	ͨ�����̴߳���������	                 ͨ�����̷߳������
���ʹ���	�̳� XxxService::Service ��ʵ�ַ���	   ͨ�� XxxService::Stub ����Զ�̷���
��������	��������	                             ���贴��/����
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

