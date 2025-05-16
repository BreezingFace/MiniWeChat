const path = require('path')
const grpc = require('@grpc/grpc-js')
const protoLoader = require('@grpc/proto-loader')

const PROTO_PATH = path.join(__dirname, 'message.proto')

const packageDefinition = protoLoader.loadSync(PROTO_PATH,{keepCase: true,longs:String,
    enums:String,defaults:true,oneofs:true})

const protoDescriptor = grpc.loadPackageDefinition(packageDefinition)

const message_proto = protoDescriptor.message

module.exports = message_proto
//加载 .proto 文件：

// PROTO_PATH 指定了 .proto 文件的路径。

// protoLoader.loadSync 用于同步加载 .proto 文件并生成包定义。

// 加载 gRPC 包定义：

// grpc.loadPackageDefinition 将加载的 .proto 文件内容转换为 gRPC 可用的包定义。

// 导出服务定义：

// message_proto 是从 .proto 文件中提取的服务定义，通过 module.exports 导出。