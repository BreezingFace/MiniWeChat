#ifndef CLICKEDLABEL_H
#define CLICKEDLABEL_H

#include <QLabel>
#include "global.h"

class ClickedLabel : public QLabel
{
    Q_OBJECT
public:
    ClickedLabel(QWidget *parent =nullptr);

    virtual void enterEvent(QEnterEvent* event) override ;

    virtual void leaveEvent(QEvent* event) override;
    virtual void mousePressEvent(QMouseEvent* event) override;
    virtual  void mouseReleaseEvent(QMouseEvent *ev) override;
    //总共6种状态，是否选中，以及鼠标是否悬浮，鼠标是否按下，所以是2*3=6种
    void SetState(QString normal="",QString hover="",QString press="",
                  QString select="",QString select_hover="",QString select_press="");

    ClickLbState GetCurState();
    bool SetCurState(ClickLbState state);
    void ResetNormalState();


private:
    QString _normal;
    QString _normal_hover;
    QString _normal_press;
    QString _selected;
    QString _selected_hover;
    QString _selected_press;
    ClickLbState _curstate;

signals:
    void clicked(QString, ClickLbState);
};





#endif // CLICKEDLABEL_H
