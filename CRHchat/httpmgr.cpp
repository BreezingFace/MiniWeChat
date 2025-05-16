#include "httpmgr.h"

HttpMgr::HttpMgr() {
    connect(this,&HttpMgr::sig_http_finish,this,&HttpMgr::slot_http_finish);

}

void HttpMgr::PostHttpReq(QUrl url, QJsonObject json, ReqId req_id, Modules mod)
{  //QByteArray表示字节数组，适合的两个主要情况是当您需要存储原始二进制数据，并且当内存保护至关重要时
    QByteArray data = QJsonDocument(json).toJson(); //将QJsonDocument数据转化为utf-8编码的json数据，
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");//application/json作为请求头，用来告诉服务端消息主体是序列化后的 JSON 字符串
    request.setHeader(QNetworkRequest::ContentLengthHeader,QByteArray::number(data.length())); //QByteArray::number（）返回一个字节数组，其中包含等价于数字n到基数的字符串（默认为10）。基数可以是2到36之间的任何值
    auto self = shared_from_this();
    QNetworkReply * reply = _manager.post(request,data);//异步地发送请求和数据，异步地得到响应
    //只要reply得到了响应，就会触发lambda表达式（槽函数）
    QObject::connect(reply,&QNetworkReply::finished,[self,reply,req_id,mod](){//这里lamdba表达式里的捕获会让self的引用计数+1，保证在connect结束后调用回调函数时HttpMgr不会被析构
        //处理错误情况
        if(reply->error()!=QNetworkReply::NoError){
            qDebug()<<reply->errorString();
            //发送信号通知完成
            emit self->sig_http_finish(req_id,"",ErrorCodes::ERR_NETWORK,mod);
            //deleteLater 是 QObject 类对象的成员函数，用于延迟删除一个 QObject 类对象，且对 QObject 类对象的删除推荐使用 deleteLater 而非 delete 。eleteLater 依赖于事件循环，调用 deleteLater 后本质是发送了一个 DeferrerDelete 事件，在事件循环处理中把对象删除。deleteLater 会在当前对象的所有事件处理完成后再删除对象
            reply->deleteLater();
            return;
        }
        //无错误
        QString res = reply->readAll();
        //发送信号通知完成
        emit self->sig_http_finish(req_id,res,ErrorCodes::SUCCESS,mod);
        reply->deleteLater();
        return;
    });//这里发送者sender和接收者receiver是同一个对象，所以可以省略第二个reply

}

HttpMgr::~HttpMgr()
{

}

void HttpMgr::slot_http_finish(ReqId id, QString res, ErrorCodes err, Modules mod)
{
    if(mod==Modules::REGISTERMOD){//注册模块
        //发送信号通知指定模块http的槽函数进行响应
        emit sig_reg_mod_finish(id,res,err);
    }
    if(mod==Modules::RESETMOD){
        emit sig_reset_mod_finish(id,res,err);
    }
    if(mod==Modules::LOGINMOD){
        emit sig_login_mod_finish(id,res,err);
    }
}
