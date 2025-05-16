#include "StatusGrpcClient.h"
#include "ConfigMgr.h"

GetChatServerRsp StatusGrpcClient::GetChatServer(int uid)
{
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
    //��ȡ����˵�ip
    std::string host = gCfgMgr["StatusServer"]["Host"];
    std::string port = gCfgMgr["StatusServer"]["Port"];
    pool_.reset(new StatusConPool(5, host, port));
}
LoginRsp StatusGrpcClient::Login(int uid, std::string token)
{
    /*
     ClientContext* context
    ���ã��ṩ���� RPC ���õ���������Ϣ�Ϳ��Ʋ���
    �����ԣ������ṩ��gRPC ���Ҫ��
    ���������ݣ�
    ��ʱ���ã�context.set_deadline(...)
    Ԫ���ݣ�headers����context.AddMetadata("key", "value")
    ѹ�����á�ƾ֤��
    �������ڣ���������� RPC �������
    �ڴ�������Ȼ��ǰû�����ö���������������Ҫ��������Ķ������������״̬
    */
    ClientContext context;
    LoginRsp reply;
    LoginReq request;
    request.set_uid(uid);
    request.set_token(token);

    auto stub = pool_->getConnection();
    /*
    LoginΪʲô��Ҫ�������������������ƽǶȣ�
    �����ע�㣺
    ClientContext�����Ƶ�����Ϊ����ε��ã�
    LoginReq��ҵ���������ݣ�����ʲô��
    LoginRsp��ҵ����Ӧ���ݣ������ʲô��
    ��չ�ԣ�
    ��ʹ��� LoginReq/LoginRsp �ܼ򵥣����Ҳ��Ҫ֧�ָ��ӳ���������ʽ���á�Ԫ���ݵȣ�
    ���ܿ��ǣ�
    ��Ӧ����ͨ��ָ�봫�ݱ��⿽��
    �����Ķ���ɸ���
    */
    Status status = stub->Login(&context, request, &reply);
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