#pragma once
#include "const.h"
#include <grpcpp/grpcpp.h> //需要使用库的接口
#include "message.grpc.pb.h"
#include "message.pb.h"
#include "Singleton.h"

using grpc::Channel;
using grpc::Status;
using grpc::ClientContext;
using message::GetChatServerReq;
using message::GetChatServerRsp;
using message::StatusService;
using message::LoginRsp;
using message::LoginReq;

/*
在这段连接池代码中：
首先创建 Channel（物理连接）
然后为每个 Channel 创建一个 Stub（服务代理）
连接池管理的是 Stub 的队列
客户端从池中获取 Stub 来发起 RPC 调用
这种分离设计的好处：
Channel 管理连接：可以复用底层连接，提高效率
Stub 管理调用：提供类型、线程安全的服务接口
连接池管理资源：控制并发连接数，避免资源耗尽
在多线程环境中，每个线程创建自己的 Stub 实例，所有线程共享底层 Channel 连接
*/
class StatusConPool {
public:
    //host:grpc服务端的ip  port:grpc服务端的端口
    StatusConPool(size_t poolSize, std::string host, std::string port)
        : poolSize_(poolSize), host_(host), port_(port), b_stop_(false) {

      
        for (size_t i = 0; i < poolSize_; ++i) {
            std::shared_ptr<Channel> channel = grpc::CreateChannel(host + ":" + port,
            grpc::InsecureChannelCredentials());//不加密连接，并实现channel的复用，保持高效的连接利用率
            connections_.push(StatusService::NewStub(channel));
        }
    }
    ~StatusConPool() {
        std::lock_guard<std::mutex> lock(mutex_);
        Close();
        while (!connections_.empty()) {
            connections_.pop();
        }
    }
    std::unique_ptr<StatusService::Stub> getConnection() {
        std::unique_lock<std::mutex> lock(mutex_);
        cond_.wait(lock, [this] {
            if (b_stop_) {
                return true;
            }
            return !connections_.empty();
            });
        //如果停止则直接返回空指针
        if (b_stop_) {
            return  nullptr;
        }
        auto context = std::move(connections_.front());
        connections_.pop();
        return context;
    }
    void returnConnection(std::unique_ptr<StatusService::Stub> context) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (b_stop_) {
            return;
        }
        connections_.push(std::move(context));
        cond_.notify_one();
    }
    void Close() {
        b_stop_ = true;
        cond_.notify_all();
    }
private:
    std::atomic<bool> b_stop_;
    size_t poolSize_;
    std::string host_;
    std::string port_;
    std::queue<std::unique_ptr<StatusService::Stub>> connections_;
    std::mutex mutex_;
    std::condition_variable cond_;
};

class StatusGrpcClient :public Singleton<StatusGrpcClient>
{
    friend class Singleton<StatusGrpcClient>;
public:
    ~StatusGrpcClient() {
    }
    GetChatServerRsp GetChatServer(int uid);//从状态服务器中获取服务ip和token
    LoginRsp Login(int uid, std::string token);
private:
    StatusGrpcClient();
    std::unique_ptr<StatusConPool> pool_;
};