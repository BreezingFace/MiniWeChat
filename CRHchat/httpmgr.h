#ifndef HTTPMGR_H
#define HTTPMGR_H
#include "singleton.h"
#include <QString>
#include <QObject>
#include <QUrl>
#include <QNetworkAccessManager>
#include <QJsonObject>
#include <QJsonDocument>


class HttpMgr: public QObject,public Singleton<HttpMgr>,public std::enable_shared_from_this<HttpMgr>//这里使用了多继承和CRTP（奇异递归模板）技术，模板在编译的时候不会被实例化，只有在执行的时候才会
{
    Q_OBJECT//只有继承了QObject类的类，才具有信号槽的能力。所以，为了使用信号槽，必须继承QObject。凡是QObject类（不管是直接子类还是间接子类），都应该在第一行代码写上Q_OBJECT。
private:
    friend class Singleton<HttpMgr>;//父类是子类的朋友，即Singleton<HttpMgr>可以访问HttpMgr的私有和保护成员
    HttpMgr();
    QNetworkAccessManager _manager;//qt用于管理网络的原生类

public:
    ~HttpMgr();//要置为公有，因为Singleton的Static成员_instance的智能指针shared_ptr最后要调用析构函数。
     void PostHttpReq(QUrl url,QJsonObject json,ReqId req_id,Modules mod);//ReqId表示请求id，Modules表示模块

private slots://槽函数
    void slot_http_finish(ReqId id,QString res,ErrorCodes err,Modules mod);//槽函数的参数不能多于信号函数
signals://信号
    void sig_http_finish(ReqId id,QString res,ErrorCodes err,Modules mod);
    void sig_reg_mod_finish(ReqId id,QString res,ErrorCodes err);
    void sig_reset_mod_finish(ReqId id,QString res,ErrorCodes err);
    void sig_login_mod_finish(ReqId id,QString res,ErrorCodes err);
};

#endif // HTTPMGR_H
