#include "global.h"
std::function <void(QWidget*)> repolish = [](QWidget* w){
    w->style()->unpolish(w);
    w->style()->polish(w);
};

std::function<QString(QString)> xorString =[](QString input){
    QString result =input ;
    int length = input.length();
    length = length % 255;
    for(int i=0;i<length;i++){
        //对每个字符进行异或操作
        result[i]=QChar(static_cast<ushort>(input[i].unicode()^static_cast<ushort>(length)));
    }

    return result;
};

QString gate_url_prefix="";
/*
std::function<void(QWidget*)>
这是一个 标准库函数对象，表示一个可以存储和调用的函数。
具体来说，它表示一个接受 QWidget* 类型指针参数、返回值为 void 的函数。

w->style()->unpolish(w);
取消小部件当前应用的样式。
这会移除现有的样式属性，比如颜色、边框等。

w->style()->polish(w);
重新应用小部件的样式。
这会重新根据当前的样式表 (QStyle) 规则绘制小部件。
*/
