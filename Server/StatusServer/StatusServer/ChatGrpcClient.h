#pragma once
#include "const.h"
#include "Singleton.h"
#include "ConfigMgr.h"
#include <grpcpp/grpcpp.h> 
#include "message.grpc.pb.h"
#include "message.pb.h"
#include <queue>
#include "data.h"
#include <json/json.h>
#include <json/value.h>
#include <json/reader.h>

using grpc::Channel;
using grpc::Status;
using grpc::ClientContext;

using message::AddFriendReq;
using message::AddFriendRsp;

using message::AuthFriendReq;
using message::AuthFriendRsp;

using message::GetChatServerRsp;
using message::LoginRsp;
using message::LoginReq;
using message::ChatService;

using message::TextChatMsgReq;
using message::TextChatMsgRsp;
using message::TextChatData;

class ChatConPool {
public:
	ChatConPool(size_t poolsize, string host, string port) :b_stop_(false), poolsize_(poolsize),host_(host),port_(port) {
		//����channel
		std::shared_ptr<Channel> channel = grpc::CreateChannel(host_ + ":" + port_, grpc::InsecureChannelCredentials());
		for (int i = 0;i < poolsize;i++) {
			connections_.push(ChatService::NewStub(channel));//ChatService::NewStub(channel) ���ص���һ�� std::unique_ptr<ChatService::Stub> ����ʱ������ֵ����C++ ���������Զ�ʶ����������������ƶ����壬����Ҫ��ʽʹ�� std::move��
		}
	}
	~ChatConPool() {
		std::lock_guard<std::mutex> lock(mutex_);
		Close();
		for (int i = 0;i < poolsize_;i++)connections_.pop();
	}
	std::unique_ptr<ChatService::Stub> getConnection() {
		std::unique_lock<std::mutex> lock(mutex_);
		cond_.wait(lock, [this]() {
			if (b_stop_)return true;
			return !connections_.empty();
			});
		if (b_stop_)return nullptr;
		auto context =std::move(connections_.front());// connections_.front() ���ص�����ֵ����,Ҫ��move תΪ��ֵ
		connections_.pop();
		return context;
	}
	void returnConnection(std::unique_ptr<ChatService::Stub> context) {
		std::unique_lock<std::mutex> lock(mutex_);
		if (!b_stop_) {
			connections_.push(std::move(context));// context ��һ��������������ֵ��
			cond_.notify_one();
		}
	}
	void Close() {
		b_stop_ = true;
		cond_.notify_all();
	}

private:
	atomic<bool> b_stop_;//�жϳ����Ƿ�ر�
	std::string host_;//�Զ˵�ip
	std::string port_;//�Զ˵Ķ˿�
	size_t poolsize_;//���ӳش�С
	std::queue<unique_ptr<ChatService::Stub>> connections_;
	std::mutex mutex_;
	std::condition_variable cond_;
};

class ChatGrpcClient : public Singleton<ChatGrpcClient>
{
	friend class Singleton<ChatGrpcClient>;
public:
	~ChatGrpcClient() {
	}
	AddFriendRsp NotifyAddFriend(std::string server_ip, const AddFriendReq& req);
	AuthFriendRsp NotifyAuthFriend(std::string server_ip, const AuthFriendReq& req);
	bool GetBaseInfo(std::string base_key, int uid, std::shared_ptr<UserInfo>& userinfo);
	TextChatMsgRsp NotifyTextChatMsg(std::string server_ip, const TextChatMsgReq& req, const Json::Value& rtvalue);
private:
	ChatGrpcClient();
	unordered_map<std::string, std::unique_ptr<ChatConPool>> _pools;
};