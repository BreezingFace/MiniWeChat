#include <iostream>
#include <json/json.h>
#include <json/value.h>
#include <json/reader.h>
#include "const.h"
#include "ConfigMgr.h"
#include "hiredis.h"
#include "RedisMgr.h"
#include "MysqlMgr.h"
#include "AsioIOServicePool.h"
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <boost/asio.hpp>
#include "StatusServiceImpl.h"
void RunServer() {
    auto& cfg = ConfigMgr::Inst();
    std::string server_address(cfg["StatusServer"]["Host"] + ":" + cfg["StatusServer"]["Port"]);
    StatusServiceImpl service;
    grpc::ServerBuilder builder;
    // 监听端口和添加服务
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    // 构建并启动gRPC服务器
    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    std::cout << "Server listening on " << server_address << std::endl;
    // 创建Boost.Asio的io_context
    boost::asio::io_context io_context;
    // 创建signal_set用于捕获SIGINT
    boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);
    // 设置异步等待SIGINT信号 signal_set 的 async_wait 会保持 io_context 运行，直到信号到来（触发 async_wait 的回调）。
    signals.async_wait([&server](const boost::system::error_code& error, int signal_number) {
        if (!error) {
            std::cout << "Shutting down server..." << std::endl;
            server->Shutdown(); // 优雅地关闭服务器
        }
        });
    // 在单独的线程中运行io_context  如果 io_context 没有工作（如没有 async_wait或其他异步操作），run() 会立即返回（因为没有任务可执行）。
    std::thread([&io_context]() { io_context.run(); }).detach();
    // 等待服务器关闭 server->Wait() 在主线程阻塞，直到 gRPC 服务器关闭（server->Shutdown(); ）。
    server->Wait();
    //io_context.stop() 在 server->Wait() 之后调用，确保 io_context 被正确停止。
    io_context.stop(); // 停止io_context
}
int main(int argc, char** argv) {
    try {
        RunServer();
    }
    catch (std::exception const& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return 0;
}