#pragma once
#include <grpcpp/grpcpp.h> //��Ҫʹ�ÿ�Ľӿ�
#include "message.grpc.pb.h"
#include "const.h"
#include "Singleton.h"

using grpc::Channel;
using grpc::Status;
using grpc::ClientContext;

using message::GetVerifyReq;
using message::GetVerifyRsp;
using message::VerifyService;

class RPConPool {
public:
	RPConPool(size_t poolsize, std::string host, std::string port):
		poolSize_(poolsize),host_(host),b_stop_(false) {
		for (size_t i = 0;i < poolSize_;++i) {
			std::shared_ptr<Channel> channel = grpc::CreateChannel(host+":"+port,
				grpc::InsecureChannelCredentials());
			//connections_.push(VerifyService::NewStub(channel));//���ù��캯������һ���������ٰѸ����ƶ����ڴ�ռ䣨1�����ع���+1���ƶ����죩
			connections_.emplace(VerifyService::NewStub(channel));//ֱ���ڶ���β�����ڴ�ռ��ڹ������Ч�ʸ�

		}
	}
	~RPConPool() {
		std::lock_guard<std::mutex> lock(mutex_);
		Close();
		while (!connections_.empty()) {
			connections_.pop();
		}
	}
	void Close() {
		b_stop_ = true;
		cond_.notify_all();
	}
	std::unique_ptr<VerifyService::Stub> getConnection() {
		std::unique_lock<std::mutex> lock(mutex_);
		cond_.wait(lock, [this]() { //���lumbda���ʽ����true����cond_�ͷ���������������ִ�У���������������������ֱ������true
			if (b_stop_) {
				return true;
			}
			return !connections_.empty();
			});
		if (b_stop_)return nullptr;

		auto connect = std::move(connections_.front());
		connections_.pop();
		return connect;

	}

	void returnConnection(std::unique_ptr<VerifyService::Stub> context) {//�黹һ��Stub�����У�ͬʱ������������һ���߳�
		std::lock_guard<std::mutex> lock(mutex_);
		if (b_stop_)return;
		connections_.push(std::move(context));
		cond_.notify_one();
   }

private:
	/*ԭ�Ӳ�����ָ��ִ�й����в��ᱻ�����߳��жϵĲ�����
	<atomic>���е�ԭ�������ṩ�������Ĳ��������ǿ��Ա�֤�ڶ��̻߳����жԹ������ݵķ����ǰ�ȫ�ġ�*/
	atomic<bool> b_stop_;
	size_t poolSize_;
	std::string host_;
	std::string port_;
	std::queue<std::unique_ptr<VerifyService::Stub>> connections_;
	std::mutex mutex_;
	std::condition_variable cond_;

};

class VerifyGrpcClient:public Singleton<VerifyGrpcClient>
{
	friend class Singleton<VerifyGrpcClient>;
public:
	GetVerifyRsp GetVerifyCode(std::string email) {
		ClientContext context;
		GetVerifyRsp reply;
		GetVerifyReq request;
		request.set_email(email);
		auto stub = pool_->getConnection();
		Status status = stub->GetVerifyCode(&context, request, &reply);
		if (status.ok())
		{
			pool_->returnConnection(std::move(stub));
			return reply;

		}
		else {
			pool_->returnConnection(std::move(stub));
			reply.set_error(ErrorCodes::RPCFailed);
			return reply;
		}
	}

private:
	VerifyGrpcClient();
	std::unique_ptr<RPConPool> pool_;
	//std::unique_ptr<VerifyService::Stub> stub_;//�൱�ڡ���ʹ����VerifyServiceͨ������ͻ���ͨ��
};

