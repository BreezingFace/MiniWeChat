// @grpc/grpc-js：gRPC 的 Node.js 实现库。
// message_proto：从 .proto 文件生成的 gRPC 服务定义。
// const_module：自定义模块，包含常量或错误码。
// uuid：用于生成唯一标识符（验证码）。
// emailModule：自定义模块，用于发送邮件。

const grpc = require('@grpc/grpc-js')
const message_proto = require('./proto')
const const_module = require('./const')
const {v4: uuidv4} = require('uuid')
const emailModule = require('./email')
const redis_module = require('./redis')
// 功能：
// 接收客户端的请求，提取邮箱地址。
// 生成一个唯一的验证码（使用 uuidv4）。
// 构造邮件内容，并通过 emailModule.SendMail 发送邮件。
// 返回结果给客户端。
// 参数：
// call：gRPC 请求对象，包含客户端传递的数据（如 call.request.email）。
// callback：gRPC 回调函数，用于返回响应。
// 逻辑：
// 使用 try-catch 捕获可能的异常。
// 如果邮件发送成功，返回 Success 状态码。
// 如果发生错误，返回 Exception 状态码。
//GetVarifyCode声明为async是为了能在内部调用await
async function GetVerifyCode(call, callback) {
    console.log("email is ", call.request.email)
    try{
        let query_ans = await redis_module.GetRedis(const_module.code_prefix + call.request.email);
        console.log("query ans is ", query_ans)
        let uniqueId = query_ans;
        if(query_ans==null){
            uniqueId = uuidv4();
            if(uniqueId.length>4)uniqueId = uniqueId.substring(0,4);
            let bres = await redis_module.SetRedisExpire(const_module.code_prefix+call.request.email,uniqueId,180)//180秒后验证码失效
            if(!bres){
                callback(null, { email:  call.request.email,
                    error:const_module.Errors.RedisErr
            });
            return;
        }
        }
        console.log("uniqueId is ", uniqueId)
        let text_str =  '您的验证码为'+ uniqueId +'请三分钟内完成注册'
        //发送邮件
        let mailOptions = {
            from: '862146685@qq.com',
            to: call.request.email,
            subject: '验证码',
            text: text_str,
        };
        let send_res = await emailModule.SendMail(mailOptions);
        console.log("send res is ", send_res)

        // gRPC 中，每个方法都需要调用 callback 来返回响应。
        // 这里的 callback 函数的第一个参数是错误对象，第二个参数是响应对象。
        //callback(null, { email: call.request.email, error: const_module.Errors.Success }); 的作用是向客户端返回成功响应。
        // 它告诉客户端：
        // 操作成功（null 表示没有错误）。
        // 返回了邮箱地址和操作状态（error: const_module.Errors.Success）。
        // 这种设计是 gRPC 服务端与客户端通信的标准方式。
        callback(null, { email:  call.request.email,
            error:const_module.Errors.Success
        }); 
    
    }catch(error){
        console.log("catch error is ", error)
        callback(null, { email:  call.request.email,
            error:const_module.Errors.Exception
        }); 
    }
}

// 功能：
// 创建一个 gRPC 服务器实例。
// 将 GetVerifyCode 方法注册到 VerifyService 服务中。
// 绑定服务器到 0.0.0.0:50051 地址，并使用非安全连接（createInsecure）。
// 启动服务器并打印日志。
function main() {
    var server = new grpc.Server()
    //结构：一个对象，键名对应 Protobuf 中定义的 RPC 方法名，键值是实际实现的函数
    //作用：将 RPC 方法名映射到服务端的具体实现函数（这里是 GetVerifyCode 函数）
    /*当客户端调用 GetVerifyCode RPC 时：
        gRPC 框架根据请求的方法名找到注册的实现函数
        自动创建 call 和 callback 参数：
        call 对象封装了客户端发送的请求数据（如 call.request.email）
        callback 函数用于回传响应
        执行你编写的 GetVerifyCode(call, callback) 函数逻辑 */
    server.addService(message_proto.VerifyService.service, { GetVerifyCode: GetVerifyCode })
    server.bindAsync('0.0.0.0:50051', grpc.ServerCredentials.createInsecure(), () => {
       // server.start()
        console.log('grpc server started')        
    })
}
main()