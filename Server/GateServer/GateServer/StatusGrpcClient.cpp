#include "StatusGrpcClient.h"
#include "ConfigMgr.h"

GetChatServerRsp StatusGrpcClient::GetChatServer(int uid)
{
    /*
     ClientContext* context
作用：提供本次 RPC 调用的上下文信息和控制参数
必需性：必须提供，gRPC 框架要求
可配置内容：
超时设置：context.set_deadline(...)
元数据（headers）：context.AddMetadata("key", "value")
压缩设置、凭证等
生命周期：必须持续到 RPC 调用完成
在代码中虽然当前没有配置额外参数，但框架需要这个上下文对象来管理调用状态
    */
    ClientContext context;
    GetChatServerRsp reply;
    GetChatServerReq request;
    request.set_uid(uid);
    auto stub = pool_->getConnection();
    Status status = stub->GetChatServer(&context, request, &reply);
    Defer defer([&stub, this]() {
        pool_->returnConnection(std::move(stub));
        });
    if (status.ok()) {
        return reply;
    }
    else {
        reply.set_error(ErrorCodes::RPCFailed);
        return reply;
    }
}
StatusGrpcClient::StatusGrpcClient()
{
    auto& gCfgMgr = ConfigMgr::Inst();
    std::string host = gCfgMgr["StatusServer"]["Host"];
    std::string port = gCfgMgr["StatusServer"]["Port"];
    pool_.reset(new StatusConPool(5, host, port));
}