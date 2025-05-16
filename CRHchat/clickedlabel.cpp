#include "clickedlabel.h"
#include <QMouseEvent>

ClickedLabel::ClickedLabel(QWidget *parent):QLabel (parent),_curstate(ClickLbState::Normal)
{
  //this->SetState("normal","hover","","selected","selected_hover","");
    setCursor(Qt::PointingHandCursor);
}

void ClickedLabel::enterEvent(QEnterEvent *event)
{

    // 在这里处理鼠标悬停进入的逻辑
    if(_curstate == ClickLbState::Normal){
        qDebug()<<"enter , change to normal hover: "<< _normal_hover;
        setProperty("state",_normal_hover);
        repolish(this);
        update();
    }else{
        qDebug()<<"enter , change to selected hover: "<< _selected_hover;
        setProperty("state",_selected_hover);
        repolish(this);
        update();
    }
    QLabel::enterEvent(event);
}

void ClickedLabel::leaveEvent(QEvent *event)
{
    // 在这里处理鼠标悬停离开的逻辑
    if(_curstate == ClickLbState::Normal){
        qDebug()<<"leave , change to normal : "<< _normal;
        setProperty("state",_normal);
        repolish(this);
        update();
    }else{
        qDebug()<<"leave , change to normal hover: "<< _selected;
        setProperty("state",_selected);
        repolish(this);
        update();
    }

    return;
    QLabel::leaveEvent(event);
}

void ClickedLabel::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        if(_curstate == ClickLbState::Normal){
           // qDebug()<<"clicked , change to selected press: "<< _selected_hover;
            qDebug()<<"PressEvent , change to selected press: "<< _selected_press;
            _curstate = ClickLbState::Selected;
            setProperty("state",_selected_press);
            repolish(this);
            update();
        }else{
           // qDebug()<<"clicked , change to normal press: "<< _normal_hover;
            qDebug()<<"PressEvent , change to normal press: "<< _normal_press;
            _curstate = ClickLbState::Normal;
            setProperty("state",_normal_press);
            repolish(this);//repolish(this) 和 update() 用于更新控件的样式和重绘控件，以确保状态变化能够立即反映在界面上。
            update();
        }
        return;
       // emit clicked();//当鼠标左键按下时，发射 clicked() 信号。这个信号可以被连接到其他槽函数，以便在点击时执行特定的操作。
    }
    // 调用基类的mousePressEvent以保证正常的事件处理
    QLabel::mousePressEvent(event);
}

void ClickedLabel::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        if(_curstate == ClickLbState::Normal){
           // qDebug()<<"clicked , change to selected hover: "<< _selected_hover;
             qDebug()<<"ReleaseEvent , change to normal hover: "<< _normal_hover;

            //setProperty("state",_selected_hover);
            setProperty("state",_normal_hover);
            repolish(this);
            update();
        }else{
             qDebug()<<"ReleaseEvent , change to select hover: "<< _selected_hover;
            //qDebug()<<"clicked , change to normal hover: "<< _normal_hover;

            //setProperty("state",_normal_hover);
            setProperty("state",_selected_hover);
            repolish(this);
            update();
        }
        emit clicked(this->text(), _curstate);
        return;
    }
    // 调用基类的mouseReleaseEvent以保证正常的事件处理
    QLabel::mouseReleaseEvent(event);
}

void ClickedLabel::SetState(QString normal, QString hover, QString press, QString select, QString select_hover, QString select_press)
{
    _normal = normal;
    _normal_hover = hover;
    _normal_press = press;
    _selected = select;
    _selected_hover = select_hover;
    _selected_press = select_press;
    setProperty("state",normal);
    repolish(this);
}

ClickLbState ClickedLabel::GetCurState()
{
    return _curstate;
}

bool ClickedLabel::SetCurState(ClickLbState state)
{
    _curstate = state;
    if (_curstate == ClickLbState::Normal) {
        setProperty("state", _normal);
        repolish(this);
    }
    else if (_curstate == ClickLbState::Selected) {
        setProperty("state", _selected);
        repolish(this);
    }

    return true;
}

void ClickedLabel::ResetNormalState()
{
    _curstate = ClickLbState::Normal;
    setProperty("state", _normal);
    repolish(this);
}
