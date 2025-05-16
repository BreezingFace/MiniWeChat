#ifndef CLICKEDBTN_H
#define CLICKEDBTN_H
#include <QPushButton>

class ClickedBtn:public QPushButton
{
    Q_OBJECT
public:
    ClickedBtn(QWidget* parent = nullptr);
    ~ClickedBtn();
    void SetState(QString normal,QString hover,QString press);
protected:
    virtual void enterEvent(QEnterEvent* event) override ;//继承自QWidget
    virtual void leaveEvent(QEvent* event) override;
    virtual void mousePressEvent(QMouseEvent *e) override;//继承自QAbstractButton
    virtual void mouseReleaseEvent(QMouseEvent *e) override;
private:
    QString _normal;
    QString _hover;
    QString _press;

};

#endif // CLICKEDBTN_H
