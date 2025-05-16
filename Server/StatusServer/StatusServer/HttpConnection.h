#pragma once
#include "const.h"

class HttpConnection:public std::enable_shared_from_this<HttpConnection>
{
public:
	friend class LogicSystem;
	//HttpConnection(tcp::socket socket);
	HttpConnection(boost::asio::io_context& ioc);
	void Start();
	tcp::socket& GetSocket() {
		return _socket;
	}
private:
	void CheckDeadline();//检查socket连接是否超时（是否在给定时间内完成数据收发）
	void WriteResponse();
	void HandleReq();
	void PreParseGetParam();
	tcp::socket _socket;
	beast::flat_buffer _buffer{ 8192 };//用来接收数据，最大长度是8192Byte
	http::request<http::dynamic_body> _request;//用于解析请求
	http::response<http::dynamic_body> _response;//用于响应客户端
	net::steady_timer deadline_{
		_socket.get_executor(),std::chrono::seconds(60)
	};//设置底层的定时器的调度器（与socket共享），定时60秒
	std::string _get_url;
	std::unordered_map<std::string, std::string> _get_params;
};

