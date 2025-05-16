#include "searchlist.h"
#include<QScrollBar>
#include "adduseritem.h"
//#include "invaliditem.h"
#include "findsuccessdlg.h"
#include "tcpmgr.h"
#include "customizeedit.h"
//#include "findfaildlg.h"
#include "loadingdlg.h"
#include <QJsonDocument>
#include <QJsonObject>
#include "findfaildlg.h"
//#include "userdata.h"
#include "usermgr.h"

SearchList::SearchList(QWidget *parent):QListWidget(parent),_find_dlg(nullptr), _search_edit(nullptr),
    _send_pending(false)//_send_pending(true)表示阻塞状态
{
    Q_UNUSED(parent);
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    // 安装事件过滤器
    this->viewport()->installEventFilter(this);
    //连接点击的信号和槽
    connect(this, &QListWidget::itemClicked, this, &SearchList::slot_item_clicked);
    //添加条目，仅作为模拟，后期需要有服务器提供数据
    addTipItem();
    //连接搜索条目
    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_user_search, this, &SearchList::slot_user_search);

}

void SearchList::CloseFindDlg()
{

}

void SearchList::SetSearchEdit(QWidget *edit)
{
    _search_edit=edit;
}

void SearchList::waitPending(bool pending)
{
    if(pending){
        _loadingDialog=new LoadingDlg(this);
        _loadingDialog->setModal(true);
        _loadingDialog->show();
        _send_pending=pending;
    }else{
        _loadingDialog->hide();
        _loadingDialog->deleteLater();
        _send_pending=pending;
    }
}
/*
添加空白提示项（invalid_item）：

高度为 10 像素，不可选中，用作视觉分隔或占位。

添加“添加用户”项（add_user_item）：

使用自定义控件 AddUserItem，大小自适应，支持交互。
*/
void SearchList::addTipItem()
{
    auto *invalid_item = new QWidget();
    QListWidgetItem *item_tmp = new QListWidgetItem;
    //qDebug()<<"chat_user_wid sizeHint is " << chat_user_wid->sizeHint();
    item_tmp->setSizeHint(QSize(250,10));
    this->addItem(item_tmp);
    invalid_item->setObjectName("invalid_item");
    this->setItemWidget(item_tmp, invalid_item);
    item_tmp->setFlags(item_tmp->flags() & ~Qt::ItemIsSelectable);//不可选中

    auto *add_user_item = new AddUserItem();
    QListWidgetItem *item = new QListWidgetItem;
    //qDebug()<<"chat_user_wid sizeHint is " << chat_user_wid->sizeHint();
    item->setSizeHint(add_user_item->sizeHint());
    this->addItem(item);
    this->setItemWidget(item, add_user_item);

}

void SearchList::slot_item_clicked(QListWidgetItem *item)
{
    QWidget *widget = this->itemWidget(item); //获取自定义widget对象
    if(!widget){
        qDebug()<< "slot item clicked widget is nullptr";
        return;
    }
    // 对自定义widget进行操作， 将item 转化为基类ListItemBase
    /*
     * qobject_cast 是 Qt 框架中提供的一种类型安全的动态转换工具，专门用于在 Qt 的对象继承体系（QObject 及其派生类）中进行类型转换。
     * 它的功能类似于 C++ 的 dynamic_cast，但相比 dynamic_cast，qobject_cast 具有更高的效率，因为它依赖于 Qt 的元对象系统（Meta-Object System），
     * 而不需要 C++ 的 RTTI（运行时类型识别）支持。
     * 对比 dynamic_cast
        特性	          qobject_cast   	     dynamic_cast
        依赖机制	     Qt 元对象系统	         C++ RTTI
        性能	       更高（无 RTTI 开销）	     较低（需运行时类型检查）
        适用范围	   仅限 QObject 派生类	     任何多态类（需虚函数）
        跨动态库	    稳定支持	                 可能失败（依赖编译器实现）
        头文件要求	需 Q_OBJECT 宏	          无特殊要求

     */
    ListItemBase *customItem = qobject_cast<ListItemBase*>(widget);
    if(!customItem){
        qDebug()<< "slot item clicked widget is nullptr";
        return;
    }
    auto itemType = customItem->GetItemType();
    if(itemType == ListItemType::INVALID_ITEM){
        qDebug()<< "slot invalid item clicked ";
        return;
    }
    /*
     * SearchInfo(int uid, QString name, QString nick, QString desc, int sex, QString icon);
    int _uid;
    QString _name;
    QString _nick;
    QString _desc;
    int _sex;
    QString _icon;
     */
    if(itemType == ListItemType::ADD_USER_TIP_ITEM){
        //todo ...
        if(_send_pending)return;

        if(!_search_edit)return;//不能让_search_list为空，否则直接返回，不让程序崩溃
        waitPending(true);
        auto search_list =  dynamic_cast<CustomizeEdit*>(_search_edit);//推荐用qobject_cast,效率更高
        auto uid_str = search_list->text();
        QJsonObject jsonObj;
        jsonObj["uid"]=uid_str;//无论是uid还是名字都存入json["uid"]中

        QJsonDocument doc(jsonObj);
        QByteArray jsonData = doc.toJson(QJsonDocument::Compact);//Json序列化
        emit TcpMgr::GetInstance()->sig_send_data(ReqId::ID_SEARCH_USER_REQ,jsonData);



        // _find_dlg = std::make_shared<FindSuccessDlg>(this);
        // //后期需要由客户端提供SearchInfo
        // auto si = std::make_shared<SearchInfo>(0,"Kevin Chen","Kv","hello , my friend!",0,":/res/head_2.jpg");
        // (std::dynamic_pointer_cast<FindSuccessDlg>(_find_dlg))->SetSearchInfo(si);
        // _find_dlg->show();
        return;
    }
    //清除弹出框
    CloseFindDlg();
}

void SearchList::slot_user_search(std::shared_ptr<SearchInfo> si)
{
    waitPending(false);
    if(si==nullptr){
        _find_dlg = std::make_shared<FindFailDlg>(this);
    }
    else{
        //如果是自己，暂且先直接返回
        auto self_uid =UserMgr::GetInstance()->GetUid();
        if(si->_uid==self_uid)return;
        //此处分两种情况，一种是搜索到了已经是自己的好友，一种是未添加好友
        //查找是否已经是好友 todo...
        bool bExist = UserMgr::GetInstance()->CheckFriendById(si->_uid);
        if(bExist){
            //如果已经是好友，进行页面跳转
            //页面跳转到指定item的会话中
            emit sig_jump_chat_item(si);
            return;
        }
        _find_dlg = std::make_shared<FindSuccessDlg>(this);
        std::dynamic_pointer_cast<FindSuccessDlg>(_find_dlg)->SetSearchInfo(si);

    }

    _find_dlg->show();

}
