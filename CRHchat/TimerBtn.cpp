#include "TimerBtn.h"
#include <QMouseEvent>
#include <QDebug>

/**
构造函数：
TimerBtn::TimerBtn(QWidget *parent): QPushButton(parent), _counter(10)：
初始化 QPushButton，并设置 _counter 的初始值为 10。
_timer = new QTimer(this)：
创建一个 QTimer 对象，并将其父对象设置为 this（即 TimerBtn），以便在 TimerBtn 销毁时自动销毁 QTimer。
连接信号与槽：
connect(_timer, &QTimer::timeout, [this](){ ... })：
将 QTimer 的 timeout 信号连接到一个 Lambda 表达式。
每次 timeout 信号触发时，_counter 减 1，并更新按钮的文本。
当 _counter 减到 0 时，停止计时器，重置 _counter，并将按钮文本设置为“获取”，同时启用按钮。
 */
TimerBtn::TimerBtn(QWidget *parent ):QPushButton(parent),_counter(10){
    _timer = new QTimer(this);

    connect(_timer,&QTimer::timeout,[this](){
        _counter--;
        if(_counter<=0){
            _timer->stop();
            _counter=10;
            this->setText("获取");
            this->setEnabled(true);
            return;
        }
        this->setText(QString::number(_counter));
    });


}

TimerBtn::~TimerBtn()
{
    _timer->stop();
}

void TimerBtn::mouseReleaseEvent(QMouseEvent *e)
{
    if(e->button()==Qt::LeftButton){
        //处理鼠标左键释放事件
        qDebug()<<"MyButton was released!";
        this->setEnabled(false);
        this->setText(QString::number(_counter));
        _timer->start(1000);//设置超时时间为1s
        emit clicked();
    }
    //调用基类的mouseReleaseEvent以确保正常的事件处理
    QPushButton::mouseReleaseEvent(e);
}
