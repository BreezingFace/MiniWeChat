#include "conuseritem.h"
#include "ui_conuseritem.h"


ConUserItem::ConUserItem(QWidget *parent):ListItemBase(parent),ui(new Ui::ConUserItem)
{
    ui->setupUi(this);
    SetItemType(ListItemType::CONTACT_USER_ITEM);
    ui->red_point->raise();//提升至z轴顶端（即置顶）
    //ui->red_point->show(true);
    ShowRedPoint(false);

}

ConUserItem::~ConUserItem()
{
    delete ui;
}

QSize ConUserItem::sizeHint() const
{
    return QSize(250,70);
}

void ConUserItem::SetInfo(std::shared_ptr<AuthInfo> auth_info)
{
    _info=std::make_shared<UserInfo>(auth_info);
    // 加载图片
    QPixmap pixmap(_info->_icon);
    // 等比例缩放图片到标签大小 KeepAspectRatio:保持图像的宽高比 SmoothTransformation:使用平滑的变换算法(双线性或双三次滤波)，这样缩放后图像质量更好
    ui->icon_lb->setPixmap(pixmap.scaled(ui->icon_lb->size(),Qt::KeepAspectRatio,Qt::SmoothTransformation));
    ui->icon_lb->setScaledContents(true);//设置QLabel自动缩放其内容以适应标签大小
    //设置姓名
    ui->user_name_lb->setText(_info->_name);
}

void ConUserItem::SetInfo(std::shared_ptr<AuthRsp> auth_rsp)
{
    _info=std::make_shared<UserInfo>(auth_rsp);
    // 加载图片
    QPixmap pixmap(_info->_icon);
    // 等比例缩放图片到标签大小 KeepAspectRatio:保持图像的宽高比 SmoothTransformation:使用平滑的变换算法(双线性或双三次滤波)，这样缩放后图像质量更好
    ui->icon_lb->setPixmap(pixmap.scaled(ui->icon_lb->size(),Qt::KeepAspectRatio,Qt::SmoothTransformation));
    ui->icon_lb->setScaledContents(true);//设置QLabel自动缩放其内容以适应标签大小
    //设置姓名
    ui->user_name_lb->setText(_info->_name);

}

void ConUserItem::SetInfo(int uid, QString name, QString icon)
{
    _info=std::make_shared<UserInfo>(uid,name,icon);
    // 加载图片
    QPixmap pixmap(_info->_icon);
    // 等比例缩放图片到标签大小 KeepAspectRatio:保持图像的宽高比 SmoothTransformation:使用平滑的变换算法(双线性或双三次滤波)，这样缩放后图像质量更好
    ui->icon_lb->setPixmap(pixmap.scaled(ui->icon_lb->size(),Qt::KeepAspectRatio,Qt::SmoothTransformation));
    ui->icon_lb->setScaledContents(true);//设置QLabel自动缩放其内容以适应标签大小
    //设置姓名
    ui->user_name_lb->setText(_info->_name);
}

void ConUserItem::ShowRedPoint(bool show)
{
    if(show){
        this->ui->red_point->show();
    }
    else{
        this->ui->red_point->hide();
    }
}
std::shared_ptr<UserInfo> ConUserItem::GetInfo()
{
    return _info;
}
