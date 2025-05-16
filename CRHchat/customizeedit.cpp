#include "customizeedit.h"

CustomizeEdit::CustomizeEdit(QWidget *parent):QLineEdit (parent),_max_len(0) //先构造基类对象，初始化_max_len，然后才是执行子类的构造函数体
{
    connect(this, &QLineEdit::textChanged, this, &CustomizeEdit::limitTextLength);
}
void CustomizeEdit::SetMaxLength(int maxLen)
{
    _max_len = maxLen;
}
