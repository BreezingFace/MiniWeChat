#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "logindialog.h"
#include "registerdialog.h"
#include "resetdialog.h"
#include "chatdialog.h"
#include "tcpmgr.h"
/***************************************************************
*  @Copyright:  Copyright (c) 2025 MQ. All rights reserved.
*  @ProjName:   %{CurrentProject:Name}
*  @FileName:   mainwindow.h
*  @Brief:
*  @Author:     陈瑞豪
*  @Date:       2025-03-04
****************************************************************/
QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void SlotSwitchReg();
    void SlotSwitchLogin();
    void SlotSwitchReset();
    void SlotSwitchLogin2();
    void SlotSwitchChat();

private:
    Ui::MainWindow *ui;
    LoginDialog* _login_dlg;
    RegisterDialog* _reg_dlg;
    ResetDialog*  _reset_dlg;
    ChatDialog* _chat_dlg;

};
#endif // MAINWINDOW_H
