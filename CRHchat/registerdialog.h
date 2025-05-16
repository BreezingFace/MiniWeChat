#ifndef REGISTERDIALOG_H
#define REGISTERDIALOG_H
#include "global.h"
//#include "logindialog.h"
#include <QDialog>

namespace Ui {
class RegisterDialog;
}

class RegisterDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RegisterDialog(QWidget *parent = nullptr);
    ~RegisterDialog();

private slots:
    void on_get_code_clicked();
    void slot_reg_mod_finish(ReqId id,QString res,ErrorCodes err);
    void on_sure_bnt_clicked();
    void on_return_btn_clicked();
    void on_cancel_bnt_clicked();

private:
    bool checkUserValid();
    bool checkEmailValid();
    bool  checkPassValid();
    bool checkConfirmValid();
    bool checkVerifyValid();
    void initHttpHandlers();
    void ChangeTipPage();
    void showTip(QString str,bool b_ok);
    Ui::RegisterDialog *ui;
    QMap<ReqId,std::function<void(const QJsonObject&)>> _handlers;
    QMap<TipErr, QString> _tip_errs;
    void AddTipErr(TipErr te,QString tips);
    void DelTipErr(TipErr te);

    QTimer* _countdown_timer;
    int _countdown;

signals:
    void sigSwitchLogin();
};

#endif // REGISTERDIALOG_H
