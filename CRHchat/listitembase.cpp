#include "listitembase.h"
#include <QStyleOption>
#include <QPainter>
ListItemBase::ListItemBase(QWidget *parent) : QWidget(parent)
{
}
void ListItemBase::SetItemType(ListItemType itemType)
{
    _itemType = itemType;
}
ListItemType ListItemBase::GetItemType()
{
    return _itemType;
}
void ListItemBase::paintEvent(QPaintEvent *event)
{   //绘制默认背景
    // 1. 初始化 QStyleOption，用于存储绘制参数
    QStyleOption opt;
    opt.initFrom(this);  // 将当前 widget 的样式状态（如 enabled、focused）复制到 opt

    // 2. 创建 QPainter，绑定到当前 widget（this）
    QPainter p(this);

    // 3. 调用当前样式主题的绘制方法，绘制窗口背景
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}
