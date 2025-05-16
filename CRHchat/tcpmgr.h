#ifndef TCPMGR_H
#define TCPMGR_H
#include <QTcpSocket>
#include "singleton.h"
#include "global.h"
#include "userdata.h"
class TcpMgr:public QObject,public Singleton<TcpMgr>, public std::enable_shared_from_this<TcpMgr>
{
    Q_OBJECT
public:
    ~TcpMgr();
private:
    friend class Singleton<TcpMgr>;
    TcpMgr();
    QTcpSocket _socket;
    QString _host;
    quint16 _port;
    quint16 _message_id;
    quint16 _message_len;
    QByteArray _buffer;
    bool _b_recv_pending;//true表示buffer还需要添加数据，即已接收的字节小于_message_len
    void initHandlers();
    void handleMsg(ReqId id,int len,QByteArray data);
    QMap<ReqId,std::function<void(ReqId id,int len,QByteArray data)>> _handlers;

signals:
    void sig_connect_success(int);
    void sig_send_data(ReqId,QByteArray);
    void sig_switch_chatdlg();
    void sig_login_failed(int);
    void sig_user_search(std::shared_ptr<SearchInfo>);
    void sig_friend_apply(std::shared_ptr<AddFriendApply>);//申请对方为好友的信号
    void sig_add_auth_friend(std::shared_ptr<AuthInfo>);//对方同意申请好友的信号
    void sig_auth_rsp(std::shared_ptr<AuthRsp>);//别人申请我为好友，我发出的信号
    void sig_text_chat_msg(std::shared_ptr<TextChatMsg> msg);
public slots:
    void slot_tcp_connect(ServerInfo);
    void slot_send_data(ReqId id,QByteArray data);

};

#endif // TCPMGR_H
