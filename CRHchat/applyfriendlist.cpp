#include "applyfriendlist.h"
#include <QWheelEvent>
#include <QScrollBar>
#include "listitembase.h"

ApplyFriendList::ApplyFriendList(QWidget *parent)
{
    Q_UNUSED(parent);
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    // 安装事件过滤器
    this->viewport()->installEventFilter(this);
}

bool ApplyFriendList::eventFilter(QObject *watched, QEvent *event)
{

    // 检查事件是否是鼠标悬浮进入或离开
    if (watched == this->viewport()) {
        if (event->type() == QEvent::Enter) {
            // 鼠标悬浮，显示滚动条
            this->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        } else if (event->type() == QEvent::Leave) {
            // 鼠标离开，隐藏滚动条
            this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        }
    }

    if (watched == this->viewport()) {
        if (event->type() == QEvent::MouseButtonPress) {
            emit sig_show_search(false);//把搜索栏隐藏
        }
    }

    // 检查事件是否是鼠标滚轮事件
    if (watched == this->viewport() && event->type() == QEvent::Wheel) {
        //使用 static_cast 进行 静态类型转换，适用于已知  event->type()确实是 Wheel 的情况,否则需要进行dynamic_cast转化
        QWheelEvent *wheelEvent = static_cast<QWheelEvent*>(event);
        int numDegrees = wheelEvent->angleDelta().y() / 8;
        int numSteps = numDegrees / 15; // 计算滚动步数

        // 设置滚动幅度
        this->verticalScrollBar()->setValue(this->verticalScrollBar()->value() - numSteps);

        return true; // 停止事件传递
    }

    return QListWidget::eventFilter(watched, event);

}


