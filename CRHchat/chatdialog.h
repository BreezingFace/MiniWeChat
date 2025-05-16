#ifndef CHATDIALOG_H
#define CHATDIALOG_H

#include <QDialog>
#include "global.h"
#include "statewidget.h"
#include "userdata.h"
#include <QListWidgetItem>
namespace Ui {
class ChatDialog;
}

class ChatDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ChatDialog(QWidget *parent = nullptr);
    ~ChatDialog();
    void addChatUserList();
    void ClearLabelState(StateWidget* lb);
    void UpdateChatMsg(std::vector<std::shared_ptr<TextChatData>> msgdata);

protected:
    bool eventFilter(QObject* watched,QEvent* event) override;
private:
    //设计模式：遵循 MVC/MVP 模式，分离界面布局与业务逻辑,ChatDialog 类通过 ui 指针访问界面组件
    void ShowSearch(bool bsearch=false);
    void AddLBGroup(StateWidget* lb);
    void handleGlobalMousePress(QMouseEvent*);
    void SetSelectChatItem(int uid = 0);
    void SetSelectChatPage(int uid = 0);
    void loadMoreConUser();
    void loadMoreChatUser();
    Ui::ChatDialog *ui;
    ChatUIMode _mode;//控制sidebar的模式（聊天模式，搜索模式）
    ChatUIMode _state;//即便在不同模式下，也有不同状态
    bool _b_loading;
    QList<StateWidget*> _lb_list;
    QMap<int,QListWidgetItem*> _chat_items_added;
    int  _cur_chat_uid;
    QWidget* _last_widget;
private slots:
    void slot_loading_chat_user();
    void slot_side_chat();
    void slot_side_contact();
    void slot_text_changed(const QString& str);
    void slot_loading_contact_user();
public slots:
    void slot_apply_friend(std::shared_ptr<AddFriendApply> apply);
    void slot_add_auth_friend(std::shared_ptr<AuthInfo> auth_info);
    void slot_auth_rsp(std::shared_ptr<AuthRsp> auth_rsp);
    void slot_jump_chat_item(std::shared_ptr<SearchInfo> si);
    void slot_friend_info_page(std::shared_ptr<UserInfo> user_info);
    void slot_switch_apply_friend_page();
    void slot_jump_chat_item_from_infopage(std::shared_ptr<UserInfo> user_info);
    void slot_item_clicked(QListWidgetItem *item);
    void slot_append_send_chat_msg(std::shared_ptr<TextChatData> msgdata);
    void slot_text_chat_msg(std::shared_ptr<TextChatMsg> msg);
};

#endif // CHATDIALOG_H
