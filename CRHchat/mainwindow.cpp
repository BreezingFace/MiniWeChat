#include "mainwindow.h"
#include "ui_mainwindow.h"
//#include "tcpmgr.h"
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    _login_dlg=new LoginDialog(this);//传入this指针，使用到了父对象机制，这样的话父对象MainWindow析构之前，子对象会析构
    _login_dlg->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);//这组指令实际上可以将自定义的widget嵌入到中心组件中。
    //如果不传入this指针，在MainWindow手动析构，也会出现进程崩溃的问题，这是因为槽函数执行setCentralWidget(_reg_dlg)后关闭注册窗口会自动析构一次_reg_dlg,主窗口又析构一次，所以就导致了进程崩溃
    setCentralWidget(_login_dlg);
   // _login_dlg->show();

    //连接登录界面注册信号
    connect(_login_dlg,&LoginDialog::switchRegister,this,&MainWindow::SlotSwitchReg);
    //连接登录界面忘记密码信号
    connect(_login_dlg, &LoginDialog::switchReset, this, &MainWindow::SlotSwitchReset);

    connect(TcpMgr::GetInstance().get(),&TcpMgr::sig_switch_chatdlg,this,&MainWindow::SlotSwitchChat);


    //emit TcpMgr::GetInstance().get()->sig_switch_chatdlg();
}

MainWindow::~MainWindow()
{
    delete ui;
    //如果不注释以下代码会导致二次析构而发生内存泄露，错误：QObject: shared QObject was deleted directly. The program is malformed and may crash
    //具体原因是Qt使用对象树管理内存，子对象由父对象销毁时自动删除。如果手动 delete 一个已有父对象的子对象，会触发这个错误。
    // if(_login_dlg){
    //     delete _login_dlg;
    //     _login_dlg=nullptr;
    // }
    // if(_reg_dlg){
    //     delete _reg_dlg;
    //     _reg_dlg=nullptr;
    // }
}

void MainWindow::SlotSwitchReg()
{
    _reg_dlg=new RegisterDialog(this);
    //Qt::CustomizeWindowHint允许自定义窗口的外观，比如手动添加按钮、标题栏等。如果不加这个标志，某些窗口样式可能无法完全自定义。
    //Qt::FramelessWindowHint移除窗口的边框和标题栏，使窗口变成无边框的样式。通常用在自定义界面，比如登录框、美观的弹窗等。
    _reg_dlg->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
    setCentralWidget(_reg_dlg);
    _login_dlg->hide();
    _reg_dlg->show();
    //连接注册界面返回登录界面
    connect(_reg_dlg,&RegisterDialog::sigSwitchLogin,this,&MainWindow::SlotSwitchLogin);

}

void MainWindow::SlotSwitchLogin()
{
    //创建一个CentralWidget, 并将其设置为MainWindow的中心部件
    _login_dlg = new LoginDialog(this);
    _login_dlg->setWindowFlags(Qt::CustomizeWindowHint|Qt::FramelessWindowHint);
    setCentralWidget(_login_dlg);
    _reg_dlg->hide();
    _login_dlg->show();
    //连接登录界面注册信号
    connect(_login_dlg, &LoginDialog::switchRegister, this, &MainWindow::SlotSwitchReg);
    //连接登录界面忘记密码信号
    connect(_login_dlg, &LoginDialog::switchReset, this, &MainWindow::SlotSwitchReset);
}

void MainWindow::SlotSwitchReset()
{
    //创建一个CentralWidget, 并将其设置为MainWindow的中心部件
    _reset_dlg = new ResetDialog(this);
    _reset_dlg->setWindowFlags(Qt::CustomizeWindowHint|Qt::FramelessWindowHint);
    setCentralWidget(_reset_dlg);//先前被设置为中心组件的_login_log将会被析构
    _login_dlg->hide();
    _reset_dlg->show();
    //注册返回登录信号和槽函数
    connect(_reset_dlg, &ResetDialog::switchLogin, this, &MainWindow::SlotSwitchLogin2);
}

void MainWindow::SlotSwitchLogin2()
{
    //创建一个CentralWidget, 并将其设置为MainWindow的中心部件
    _login_dlg = new LoginDialog(this);
    _login_dlg->setWindowFlags(Qt::CustomizeWindowHint|Qt::FramelessWindowHint);
    setCentralWidget(_login_dlg);
    _reset_dlg->hide();
    _login_dlg->show();
    //连接登录界面忘记密码信号
    connect(_login_dlg, &LoginDialog::switchReset, this, &MainWindow::SlotSwitchReset);
    //连接登录界面注册信号
    connect(_login_dlg, &LoginDialog::switchRegister, this, &MainWindow::SlotSwitchReg);
}

void MainWindow::SlotSwitchChat()
{
    _chat_dlg=new ChatDialog();
    // Qt::CustomizeWindowHint：去掉标题栏,但保留边框阴影;用户可以通过窗口的标题栏上的“自定义”按钮来打开窗口的定制化选项，选择隐藏或显示窗口的最大化、最小化和关闭按钮，以及调整窗口的大小和位置等选项
    // Qt::FramelessWindowHint：移除窗口边框
    //效果：创建一个无边框自定义窗口（常见于聊天软件皮肤化设计）
    _chat_dlg->setWindowFlags(Qt::CustomizeWindowHint|Qt::FramelessWindowHint);
    setCentralWidget(_chat_dlg);
    _login_dlg->hide();
    _chat_dlg->show();
    this->setMinimumSize(QSize(800,600));
    this->setMaximumSize(QWIDGETSIZE_MAX,QWIDGETSIZE_MAX);

}
