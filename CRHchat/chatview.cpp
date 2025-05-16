#include "chatview.h"
#include <QScrollBar>
#include <QStyle>
#include <QEvent>
#include <QStyleOption>
#include <QPainter>
/*
+-----------------------------------------------+
| ChatView (QWidget)                            |
|   (边距: 0,0,0,0)                            |
| +-------------------------------------------+ |
| | QVBoxLayout (pMainLayout)                 | |
| |                                           | |
| | +---------------------------------------+ | |
| | | QScrollArea (m_pScrollArea)           | | |
| | |   objectName: "chat_area"             | | |
| | | +-----------------------------------+ | | |
| | | | QWidget (w)                       | | | |
| | | |   objectName: "chat_bg"           | | | |
| | | |   autoFillBackground: true        | | | |
| | | | +-------------------------------+ | | | |
| | | | | QVBoxLayout (pVLayout_1)      | | | | |
| | | | |                               | | | | |
| | | | | +---------------------------+ | | | | |
| | | | | | QWidget (占位部件)         | | | | | |
| | | | | |   (伸缩因子: 100000)       | | | | | |
| | | | | +---------------------------+ | | | | |
| | | | |                               | | | | |
| | | | +-------------------------------+ | | | |
| | | +-----------------------------------+ | | |
| | |                                         | |
| | | +-------------------------------------+ | |
| | | | QHBoxLayout (pHLayout_2)            | | |
| | | |   (边距: 0,0,0,0)                   | | |
| | | |                                     | | |
| | | | [空]                       QScrollBar| | |
| | | |                             (右对齐) | | |
| | | +-------------------------------------+ | |
| | +---------------------------------------+ | |
| +-------------------------------------------+ |
+-----------------------------------------------+
ChatView
├── VBoxLayout (根布局)
│   └── QScrollArea (主滚动区域)
│       ├── HBoxLayout (滚动条专用布局)
│       │   └── QScrollBar (右侧对齐)
│       └── QWidget (内容容器)
│           └── VBoxLayout (消息布局)
│               ├── [动态添加的消息项...]
│               └── QWidget (占位弹簧)
这种嵌套布局是为了同时满足以下核心功能需求：
可滚动区域：需要展示可能超出可视范围的长聊天内容
自定义滚动条：需要将滚动条从默认位置移到右侧独立区域
动态消息布局：需要支持消息从底部向上堆叠的特殊布局方式
样式定制能力：需要为每个可定制部分提供独立的样式控制点
*/
ChatView::ChatView(QWidget* parent):QWidget(parent),isAppended(false) {
    QVBoxLayout *pMainLayout = new QVBoxLayout();
    this->setLayout(pMainLayout);
   // pMainLayout->setMargin(0);
    pMainLayout->setContentsMargins(0, 0, 0, 0);  // 左、上、右、下边距均为 0
    m_pScrollArea = new QScrollArea();
    m_pScrollArea->setObjectName("chat_area");
    pMainLayout->addWidget(m_pScrollArea);
    QWidget *w = new QWidget(this);
    w->setObjectName("chat_bg");
    w->setAutoFillBackground(true);// 确保背景可被样式表或调色板填充
    QVBoxLayout *pVLayout_1 = new QVBoxLayout();
    pVLayout_1->addWidget(new QWidget(), 100000);//100000是伸缩因子
    w->setLayout(pVLayout_1);
    m_pScrollArea->setWidget(w);

    m_pScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    QScrollBar *pVScrollBar = m_pScrollArea->verticalScrollBar();
    connect(pVScrollBar, &QScrollBar::rangeChanged,this, &ChatView::onVScrollBarMoved);//滚动条的范围一变化，就会触发槽函数

    // 创建水平布局 pHLayout_2
    // 将滚动条添加到布局中（右对齐，伸缩因子为0）
    // 设置布局边距为0
    // 将布局设置给滚动区域（覆盖默认布局）
    // 初始时隐藏滚动条
    QHBoxLayout *pHLayout_2 = new QHBoxLayout();
    pHLayout_2->addWidget(pVScrollBar, 0, Qt::AlignRight);//伸缩因子设为0
    pHLayout_2->setContentsMargins(0,0,0,0);
    m_pScrollArea->setLayout(pHLayout_2);
    pVScrollBar->setHidden(true);

    m_pScrollArea->setWidgetResizable(true);
    m_pScrollArea->installEventFilter(this);
    initStyleSheet();
}

/*
占位部件的作用：
确保新消息总是添加到现有消息和占位部件之间
保持占位部件在最底部，使聊天区域可以向上扩展而不是向下堆积
消息顺序：
最新消息会出现在布局的较低位置（视觉上的下方）
符合大多数聊天应用的显示习惯（新消息在底部）
自动扩展：
由于占位部件有大的伸缩因子，当消息增加时，它会压缩自己的空间
确保消息区域可以自动扩展而不需要手动调整大小
*/
void ChatView::appendChatItem(QWidget *item)
{
    // 1. 获取滚动区域内部部件w的垂直布局pVLayout_1
    QVBoxLayout *vl = qobject_cast<QVBoxLayout *>(m_pScrollArea->widget()->layout());

    // 2. 在布局的倒数第二个位置（占位部件之前）插入新部件
    vl->insertWidget(vl->count()-1, item);

    // 3. 设置追加标志
    isAppended = true;
}

void ChatView::prependChatItem(QWidget *item)
{

}

void ChatView::insertChatItem(QWidget *before, QWidget *item)
{

}

void ChatView::removeAllItem()
{
    QVBoxLayout *layout = qobject_cast<QVBoxLayout *>(m_pScrollArea->widget()->layout());

    int count = layout->count();

    for (int i = 0; i < count - 1; ++i) {
        QLayoutItem *item = layout->takeAt(0); // 始终从第一个控件开始删除
        if (item) {
            if (QWidget *widget = item->widget()) {
                delete widget;
            }
            delete item;
        }
    }

}

bool ChatView::eventFilter(QObject *o, QEvent *e)
{
    if(e->type() == QEvent::Enter && o == m_pScrollArea)
    { //如果垂直滚动条的最大长度为0（没有多余数据无法展示），则不显示滚动条，否则显示
        m_pScrollArea->verticalScrollBar()->setHidden(m_pScrollArea->verticalScrollBar()->maximum() == 0);
    }
    else if(e->type() == QEvent::Leave && o == m_pScrollArea)
    {
        m_pScrollArea->verticalScrollBar()->setHidden(true);
    }
    return QWidget::eventFilter(o, e);
}

void ChatView::paintEvent(QPaintEvent *event)
{
    //绘制标准背景
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}
//确保在添加新内容后，滚动条自动滚动到底部。
void ChatView::onVScrollBarMoved(int min, int max)
{
    if(isAppended) //添加item可能调用多次
    {
        QScrollBar *pVScrollBar = m_pScrollArea->verticalScrollBar();
        pVScrollBar->setSliderPosition(pVScrollBar->maximum());
        // 500ms 后重置 isAppended，在这500ms内，滚动条不会滚动
        /*
        isAppended 标志：
        用于避免在短时间内多次触发滚动（例如连续添加多条消息）。
        如果 isAppended 为 true，表示当前正在追加内容，直接滚动到底部。
        */
        QTimer::singleShot(500, [this]()
                           {
                               isAppended = false;
                           });
    }
}

void ChatView::initStyleSheet()
{

}
