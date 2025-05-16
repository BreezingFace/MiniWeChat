#include "chatitembase.h"
/*核心组件
子控件初始化：
m_pNameLabel：显示用户名的标签，设置字体为 9pt 微软雅黑，固定高度 20px。
m_pIconLabel：用户头像标签，固定尺寸 42x42px，允许缩放内容。
m_pBubble：消息气泡（实际内容容器），暂未填充具体内容。
QGridLayout：网格布局管理器，控制各控件的位置和间距。
布局策略：
全局边距和间距均为 3px，保持紧凑但不过于拥挤。
添加一个水平扩展的 QSpacerItem，用于动态填充剩余空间。*/
ChatItemBase::ChatItemBase(ChatRole role, QWidget *parent)
    : QWidget(parent)
    , m_role(role)
{
    m_pNameLabel    = new QLabel();
    m_pNameLabel->setObjectName("chat_user_name");
    QFont font("Microsoft YaHei");
    font.setPointSize(9);
    m_pNameLabel->setFont(font);
    m_pNameLabel->setFixedHeight(20);
    m_pIconLabel    = new QLabel();
    m_pIconLabel->setScaledContents(true);
    m_pIconLabel->setFixedSize(42, 42);
    m_pBubble       = new QWidget();
    QGridLayout *pGLayout = new QGridLayout();
    pGLayout->setVerticalSpacing(3);
    pGLayout->setHorizontalSpacing(3);
    pGLayout->setContentsMargins(3,3,3,3);
    QSpacerItem*pSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    if(m_role == ChatRole::Self)
    {/*
1. 自己发送的消息（右侧对齐）
用户名：右对齐，右侧保留 8px 边距。
布局结构：
plaintext
复制
[空白占位]  [用户名]  [头像]
[空白占位]  [气泡]    [头像]
列权重：
第 0 列（左侧空白）占 2 份空间。
第 1 列（用户名和气泡）占 3 份空间。
第 2 列（头像）固定宽度。
        */
        m_pNameLabel->setContentsMargins(0,0,8,0);
        m_pNameLabel->setAlignment(Qt::AlignRight);
        pGLayout->addWidget(m_pNameLabel, 0,1, 1,1);
        pGLayout->addWidget(m_pIconLabel, 0, 2, 2,1, Qt::AlignTop);
        pGLayout->addItem(pSpacer, 1, 0, 1, 1);
        pGLayout->addWidget(m_pBubble, 1,1, 1,1);
        //把第0列和第1列的宽度比设置为2：3
        pGLayout->setColumnStretch(0, 2);
        pGLayout->setColumnStretch(1, 3);
    }else{
        m_pNameLabel->setContentsMargins(8,0,0,0);
        m_pNameLabel->setAlignment(Qt::AlignLeft);
        pGLayout->addWidget(m_pIconLabel, 0, 0, 2,1, Qt::AlignTop);
        pGLayout->addWidget(m_pNameLabel, 0,1, 1,1);
        pGLayout->addWidget(m_pBubble, 1,1, 1,1);
        pGLayout->addItem(pSpacer, 2, 2, 1, 1);
        pGLayout->setColumnStretch(1, 3);
        pGLayout->setColumnStretch(2, 2);
    }
    this->setLayout(pGLayout);
}
void ChatItemBase::setUserName(const QString &name)
{
    m_pNameLabel->setText(name);
}
void ChatItemBase::setUserIcon(const QPixmap &icon)
{
    m_pIconLabel->setPixmap(icon);
}
/*功能说明
作用：
将原有的 m_pBubble（消息气泡容器）替换为新的自定义控件 w，并自动释放旧气泡的内存。
关键操作：
通过 QGridLayout::replaceWidget() 无缝替换控件。
删除旧气泡（delete m_pBubble），避免内存泄漏。
更新成员变量 m_pBubble 指向新控件。*/
void ChatItemBase::setWidget(QWidget *w)
{
    QGridLayout *pGLayout = (qobject_cast<QGridLayout *>)(this->layout());
    pGLayout->replaceWidget(m_pBubble, w);
    delete m_pBubble;
    m_pBubble = w;
}
