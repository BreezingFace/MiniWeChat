#include "chatuserlist.h"
#include "usermgr.h"
#include <QTimer>
#include <QCoreApplication>
ChatUserList::ChatUserList(QWidget *parent):QListWidget(parent),_load_pending(false)
{
    Q_UNUSED(parent);
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);//始终禁用水平滚动条
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    // 安装事件过滤器
    //在构造函数中通过this->viewport()->installEventFilter(this)安装后：
   // 所有发送到viewport()的事件会先经过eventFilter方法
   //  然后再决定是否传递给原始目标对象
    this->viewport()->installEventFilter(this);//对 viewport() 安装事件过滤器，用于监听视口区域的事件.视口是 QListWidget 中实际显示内容的区域
}
/*
通过事件过滤器实现了：
智能滚动条显示/隐藏
自定义滚轮滚动行为
滚动到底部自动加载功能
事件传递机制
[事件发生] → [操作系统捕获] → [Qt事件系统] → [事件过滤器eventFilter]
    ↓ (return false)
[目标对象的原生事件处理函数] (如mousePressEvent等)
    ↓
[父控件的事件处理] (可选)
    ↓
[最终未被处理则丢弃]
*/
bool ChatUserList::eventFilter(QObject *watched, QEvent *event)
{
    // 检查事件是否是鼠标悬浮进入或离开
    if (watched == this->viewport()) {
        if (event->type() == QEvent::Enter) {
            // 鼠标悬浮，显示滚动条
            this->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
            // 这里没有return语句，相当于return false 事件继续传递给QListWidget本身的enterEvent QListWidget可能会处理一些默认的悬停效果
        } else if (event->type() == QEvent::Leave) {
            // 鼠标离开，隐藏滚动条
            this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        }
    }
    // 检查事件是否是鼠标滚轮事件
    if (watched == this->viewport() && event->type() == QEvent::Wheel) {
        QWheelEvent *wheelEvent = static_cast<QWheelEvent*>(event);
        int numDegrees = wheelEvent->angleDelta().y() / 8;
        int numSteps = numDegrees / 15; // 计算滚动步数
        // 设置滚动幅度
        this->verticalScrollBar()->setValue(this->verticalScrollBar()->value() - numSteps);
        // 检查是否滚动到底部
        QScrollBar *scrollBar = this->verticalScrollBar();
        int maxScrollValue = scrollBar->maximum();
        int currentValue = scrollBar->value();
        //int pageSize = 10; // 每页加载的联系人数量
        if (maxScrollValue - currentValue <= 0) {
            auto b_loaded = UserMgr::GetInstance()->IsLoadChatFin();
            if(b_loaded){
                return true;
            }

            if(_load_pending){
                return true;
            }
            // 滚动到底部，加载新的联系人
            qDebug()<<"load more chat user";
            _load_pending = true;

            QTimer::singleShot(100, [this](){
                _load_pending = false;
                QCoreApplication::quit(); // 完成后退出应用程序
            });
            //发送信号通知聊天界面加载更多聊天内容
            emit sig_loading_chat_user();
        }
        return true; // 停止事件传递 直接终止，不会调用QListWidget的默认滚轮处理
    }
    return QListWidget::eventFilter(watched, event);
}
