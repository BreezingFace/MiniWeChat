// 引入依赖：
// nodemailer 是一个流行的 Node.js 库，用于发送电子邮件。
// config_module 是一个自定义模块，用于加载发送方邮箱地址和授权码。
// 创建邮件传输代理：
// nodemailer.createTransport 用于创建一个邮件传输代理。
// 配置项包括：
// host: QQ 邮箱的 SMTP 服务器地址（smtp.qq.com）。
// port: SMTP 服务器端口（465）
// secure: 是否使用 SSL/TLS（true）。
// auth: 包含发送方邮箱地址和授权码
const nodemailer = require('nodemailer');
const config_module = require("./config");
/**
 * 创建发送邮件的代理
 */
let transport = nodemailer.createTransport({
    host: 'smtp.qq.com',
    port: 465,
    secure: true,
    auth: {
        user: config_module.email_user, // 发送方邮箱地址
        pass: config_module.email_pass // 邮箱授权码或者密码
    }
});

/**
 * 发送邮件的函数 该函数的主要作用是通过 nodemailer 发送邮件，并将发送结果封装为 Promise，方便调用者使用异步编程模式（如 async/await 或 .then()/.catch()）。
 * 因为transport.SendMail相当于一个异步函数，调用该函数后发送的结果是通过回调函数通知的，所以我们没办法同步使用，需要用Promise封装这个调用，抛出Promise给外部，那么外部就可以通过await或者then catch的方式处理了
 * @param {*} mailOptions_ 发送邮件的参数
 * @returns 
 */
function SendMail(mailOptions_){
    return new Promise(function(resolve, reject){
        transport.sendMail(mailOptions_, function(error, info){
            if (error) {
                console.log(error);
                reject(error);
            } else {
                console.log('邮件已成功发送：' + info.response);
                resolve(info.response)
            }
        });
    })
}
module.exports.SendMail = SendMail