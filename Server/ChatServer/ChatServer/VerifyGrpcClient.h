#pragma once
#include <grpcpp/grpcpp.h> //需要使用库的接口
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
			//connections_.push(VerifyService::NewStub(channel));//调用构造函数创建一个副本，再把副本移动到内存空间（1次重载构造+1次移动构造）
			connections_.emplace(VerifyService::NewStub(channel));//直接在队列尾部的内存空间内构造对象，效率高

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
		cond_.wait(lock, [this]() { //如果lumbda表达式返回true，则cond_释放锁，并继续往下执行；否则阻塞，并持有锁，直到返回true
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

	void returnConnection(std::unique_ptr<VerifyService::Stub> context) {//归还一个Stub给队列，同时条件变量唤醒一个线程
		std::lock_guard<std::mutex> lock(mutex_);
		if (b_stop_)return;
		connections_.push(std::move(context));
		cond_.notify_one();
   }

private:
	/*原子操作是指在执行过程中不会被其他线程中断的操作。
	<atomic>库中的原子类型提供了这样的操作，它们可以保证在多线程环境中对共享数据的访问是安全的。*/
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
	//std::unique_ptr<VerifyService::Stub> stub_;//相当于“信使”，VerifyService通过它与客户端通信
};

