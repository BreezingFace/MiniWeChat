#include "registerdialog.h"
#include "ui_registerdialog.h"
//#include "global.h"
#include "httpmgr.h"
#include <QDebug>

/*
 * 初始化注册对话框的 UI 元素。
    设置密码和确认密码框为密码模式，输入时显示圆点。
        初始化错误提示的属性。
            连接 HttpMgr 的信号 sig_reg_mod_finish 到注册完成的槽函数。
                调用 initHttpHandlers 初始化 HTTP 请求处理器。
 */
RegisterDialog::RegisterDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::RegisterDialog),_countdown(5)
{
    ui->setupUi(this);
    ui->pass_edit->setEchoMode(QLineEdit::Password);//密码模式
    ui->confirm_edit->setEchoMode(QLineEdit::Password);

    ui->err_tip->setProperty("state","normal");//告诉qt err_tip的初始属性
    repolish(ui->err_tip);


    connect(HttpMgr::GetInstance().get(),&HttpMgr::sig_reg_mod_finish,this,&RegisterDialog::slot_reg_mod_finish);//QObject::connect函数异步执行，它的作用就是将发射者 sender 对象中的信号 signal 与接收者 receiver 中的 member 槽函数联系起来
   // connect(ui->get_code, &QPushButton::clicked, this, &RegisterDialog::on_get_code_clicked);
    initHttpHandlers();

    ui->err_tip->clear();

    //day11 设定输入框输入后清空字符串
    ui->err_tip->clear();
    connect(ui->user_edit,&QLineEdit::editingFinished,this,[this](){
        checkUserValid();
    });
    connect(ui->email_edit, &QLineEdit::editingFinished, this, [this](){
        checkEmailValid();
    });
    connect(ui->pass_edit, &QLineEdit::editingFinished, this, [this](){
        checkPassValid();
    });
    connect(ui->confirm_edit, &QLineEdit::editingFinished, this, [this](){
        checkConfirmValid();
    });
    connect(ui->verify_edit, &QLineEdit::editingFinished, this, [this](){
        checkVerifyValid();
    });

    ui->pass_visible->setCursor(Qt::PointingHandCursor);
    ui->confirm_visible->setCursor(Qt::PointingHandCursor);

    ui->pass_visible->SetState("unvisible","unvisible_hover","","visible",
                               "visible_hover","");
    ui->confirm_visible->SetState("unvisible","unvisible_hover","","visible",
                                  "visible_hover","");

    //连接点击事件

    /*
     * 当用户点击 ui->pass_visible 控件时，触发 ClickedLabel::clicked 信号。

        根据 ui->pass_visible 的当前状态，切换 ui->pass_edit 的显示模式：

        如果状态是 Normal，设置为密码模式。

        如果状态是 Selected，设置为普通文本模式。
     */
    connect(ui->pass_visible, &ClickedLabel::clicked, this, [this]() {
        auto state = ui->pass_visible->GetCurState();
        if(state == ClickLbState::Normal){
            ui->pass_edit->setEchoMode(QLineEdit::Password);
        }else{
            ui->pass_edit->setEchoMode(QLineEdit::Normal);
        }
        qDebug() << "Label was clicked!";
    });
    connect(ui->confirm_visible, &ClickedLabel::clicked, this, [this]() {
        auto state = ui->confirm_visible->GetCurState();
        if(state == ClickLbState::Normal){
            ui->confirm_edit->setEchoMode(QLineEdit::Password);
        }else{
            ui->confirm_edit->setEchoMode(QLineEdit::Normal);
        }
        qDebug() << "Label was clicked!";
    });
    // 创建定时器
    _countdown_timer = new QTimer(this);
    // 连接信号和槽
    connect(_countdown_timer, &QTimer::timeout, this,[this](){
        if(_countdown==0){
            _countdown_timer->stop();
            emit sigSwitchLogin();
            return;
        }
        _countdown--;
        /*
        "注册成功，%1 s后返回登录" 是一个格式化字符串，其中 %1 是一个占位符，表示这里会被一个动态值替换。
        .arg(_countdown) 是 QString 的成员函数 arg()，用于将 %1 替换为 _countdown 的值。
        */
        auto str = QString("注册成功，%1秒后返回登录").arg(_countdown);
        ui->tip_lb->setText(str);
    });

}

RegisterDialog::~RegisterDialog()
{
    qDebug()<<"destruct RegisterDialog";
    delete ui;
}

/*
获取邮箱输入框的文本。
用正则表达式验证邮箱格式。
如果格式正确，显示成功提示（实际应用中这里应该调用 HTTP 请求发送验证码）。
如果格式错误，显示错误提示。*/
void RegisterDialog::on_get_code_clicked()
{
    auto email=ui->email_edit->text();//获取到邮箱一栏的文本
    QRegularExpression regex(R"((\w+)(\.|_)?(\w*)@(\w+)(\.(\w+))+)");//正则表达式
    bool match = regex.match(email).hasMatch();
    if(match){
        //发送http请求获取验证码
       // showTip(tr("邮箱地址正确"),true);
        QJsonObject json_obj;
        json_obj["email"]=email;
        HttpMgr::GetInstance()->PostHttpReq(QUrl(gate_url_prefix+"/get_verifycode"),json_obj,ReqId::ID_GET_VERIFY_CODE,
                                            Modules::REGISTERMOD);
    }
    else{
        showTip(tr("邮箱地址不正确"),false);
    }
}

/*
 * 处理 HTTP 请求完成后的响应。
如果请求失败，显示网络错误提示。
如果返回的字符串不能解析为 JSON 或不是对象，显示解析失败提示。
如果解析成功，根据请求 ID 执行相应的回调函数。
 */
void RegisterDialog::slot_reg_mod_finish(ReqId id, QString res, ErrorCodes err)
{
    if(err!=ErrorCodes::SUCCESS){
        showTip(tr("网络请求错误"),false);//tr函数用来实现国际化，实现语言的自动翻译
        return ;
    }
    //解析JSON 字符串，res 转化为QByteArray
    QJsonDocument jsonDoc = QJsonDocument::fromJson(res.toUtf8());//将QString转为QByteArray,可以将jsonDoc理解为json文件
    if(jsonDoc.isNull()){
        showTip(tr("json解析失败"),false);
        return;

    }
    //json 解析错误
    if(!jsonDoc.isObject()){
        showTip(tr("json解析失败"),false);
        return;
    }

    //
    _handlers[id](jsonDoc.object());//调用id对应的回调函数

    return;



}

/*
 * 初始化 HTTP 请求处理器。
处理获取验证码的响应：
如果返回的 error 字段不是成功状态，显示参数错误提示。
如果成功，显示验证码已发送的提示，并打印邮箱地址。
 */
void RegisterDialog::initHttpHandlers()
{
    //注册获取验证码回包的逻辑
    _handlers.insert(ReqId::ID_GET_VERIFY_CODE,[this](const QJsonObject& jsonObj){
       // qDebug() << "Callback function called";
        int error = jsonObj["error"].toInt();
        if(error!=ErrorCodes::SUCCESS){
            showTip(tr("参数错误"),false);
            return;
        }
        auto email = jsonObj["email"].toString();
        showTip(tr("验证码已经发送到邮箱，注意查收"),true);
        qDebug()<<"email is"<<email;

    });
    //注册注册用户回包逻辑
    _handlers.insert(ReqId::ID_REG_USER, [this](const QJsonObject& jsonObj){
        int error = jsonObj["error"].toInt();
        if(error != ErrorCodes::SUCCESS){
            showTip(tr("参数错误"),false);
            return;
        }
        auto email = jsonObj["email"].toString();
        showTip(tr("用户注册成功"), true);
        qDebug() << "user uid is " <<jsonObj["uid"];//.toString()
        qDebug()<< "email is " << email ;
        ChangeTipPage();
    });
}

void RegisterDialog::ChangeTipPage()
{
    _countdown_timer->stop();
    ui->stackedWidget->setCurrentWidget(ui->page_2);//切换到页面2
    // 启动定时器，设置间隔为1000毫秒（1秒）
    _countdown_timer->start(1000);
}

void RegisterDialog::showTip(QString str,bool b_ok){
    if(b_ok){
           ui->err_tip->setProperty("state","normal");
    }
    else{
         ui->err_tip->setProperty("state","err");
    }
    ui->err_tip->setText(str);

    repolish(ui->err_tip);
}

void RegisterDialog::AddTipErr(TipErr te, QString tips)
{
    _tip_errs[te] = tips;
    showTip(tips, false);
}

void RegisterDialog::DelTipErr(TipErr te)
{
    _tip_errs.remove(te);
    if(_tip_errs.empty()){
        ui->err_tip->clear();
        return;
    }
    showTip(_tip_errs.first(), false);
}


void RegisterDialog::on_sure_bnt_clicked()
{
    if(ui->user_edit->text() == ""){
        showTip(tr("用户名不能为空"), false);
        return;
    }
    if(ui->email_edit->text() == ""){
        showTip(tr("邮箱不能为空"), false);
        return;
    }
    if(ui->pass_edit->text() == ""){
        showTip(tr("密码不能为空"), false);
        return;
    }
    if(ui->confirm_edit->text() == ""){
        showTip(tr("确认密码不能为空"), false);
        return;
    }
    if(ui->confirm_edit->text() != ui->pass_edit->text()){
        showTip(tr("密码和确认密码不匹配"), false);
        return;
    }
    if(ui->verify_edit->text() == ""){
        showTip(tr("验证码不能为空"), false);
        return;
    }
    //day11 发送http请求注册用户
    QJsonObject json_obj;
    json_obj["user"] = ui->user_edit->text();
    json_obj["email"] = ui->email_edit->text();
    json_obj["passwd"] = xorString(ui->pass_edit->text());
    json_obj["confirm"] = xorString(ui->confirm_edit->text());
    json_obj["verifycode"] = ui->verify_edit->text();//'varifycode'
    HttpMgr::GetInstance()->PostHttpReq(QUrl(gate_url_prefix+"/user_register"),
                                        json_obj, ReqId::ID_REG_USER,Modules::REGISTERMOD);
}

bool RegisterDialog::checkUserValid()
{
    if(ui->user_edit->text() == ""){
        AddTipErr(TipErr::TIP_USER_ERR, tr("用户名不能为空"));
        return false;
    }
    DelTipErr(TipErr::TIP_USER_ERR);
    return true;
}

bool RegisterDialog::checkEmailValid()
{
    auto email=ui->email_edit->text();//获取到邮箱一栏的文本
    QRegularExpression regex(R"((\w+)(\.|_)?(\w*)@(\w+)(\.(\w+))+)");//正则表达式
    bool match = regex.match(email).hasMatch();
    if(!match){
       //提示邮箱不正确
        AddTipErr(TipErr::TIP_EMAIL_ERR,tr("邮箱地址不正确"));
       return false;
    }
    DelTipErr(TipErr::TIP_EMAIL_ERR);
    return true;
}

bool RegisterDialog::checkPassValid()
{
    auto pass = ui->pass_edit->text();
    if(pass.length() < 6 || pass.length()>15){
        //提示长度不准确
        AddTipErr(TipErr::TIP_PWD_ERR, tr("密码长度应为6~15"));
        return false;
    }
    // 创建一个正则表达式对象，按照上述密码要求
    // 这个正则表达式解释：
    // ^[a-zA-Z0-9!@#$%^&*]{6,15}$ 密码长度至少6，可以是字母、数字和特定的特殊字符
    QRegularExpression regExp("^[a-zA-Z0-9!@#$%^&*]{6,15}$");
    bool match = regExp.match(pass).hasMatch();
    if(!match){
        //提示字符非法
        AddTipErr(TipErr::TIP_PWD_ERR, tr("不能包含非法字符"));
        return false;;
    }
    DelTipErr(TipErr::TIP_PWD_ERR);
    return true;
}

bool RegisterDialog::checkVerifyValid()
{
    auto pass = ui->verify_edit->text();
    if(pass.isEmpty()){
        AddTipErr(TipErr::TIP_VERIFY_ERR, tr("验证码不能为空"));
        return false;
    }
    DelTipErr(TipErr::TIP_VERIFY_ERR);
    return true;
}

bool RegisterDialog::checkConfirmValid(){


    auto pass = ui->pass_edit->text();
    auto confirm = ui->confirm_edit->text();
    // if(confirm.length() < 6 || confirm.length()>15){
    //     //提示长度不准确
    //     AddTipErr(TipErr::TIP_CONFIRM_ERR, tr("密码长度应为6~15"));
    //     return false;
    // }
    // // 创建一个正则表达式对象，按照上述密码要求
    // // 这个正则表达式解释：
    // // ^[a-zA-Z0-9!@#$%^&*]{6,15}$ 密码长度至少6，可以是字母、数字和特定的特殊字符
    // QRegularExpression regExp("^[a-zA-Z0-9!@#$%^&*]{6,15}$");
    // bool match = regExp.match(confirm).hasMatch();
    // if(!match){
    //       //提示字符非法
    //       AddTipErr(TipErr::TIP_CONFIRM_ERR, tr("不能包含非法字符"));
    //        return false;;
    //  }
    //DelTipErr(TipErr::TIP_CONFIRM_ERR);
     if(pass!=confirm){
         //提示密码不匹配
         AddTipErr(TipErr::TIP_CONFIRM_ERR,tr("确认密码和密码不匹配"));
         return false;
     }
     DelTipErr(TipErr::TIP_CONFIRM_ERR);
     return true;

}

void RegisterDialog::on_return_btn_clicked()
{
    _countdown_timer->stop();
    emit sigSwitchLogin();
}


void RegisterDialog::on_cancel_bnt_clicked()
{
    _countdown_timer->stop();
    emit sigSwitchLogin();
}

