#pragma once
#include "const.h"

class CServer:public std::enable_shared_from_this<CServer>
{
public:
	CServer(boost::asio::io_context& ioc, unsigned short& port);//ioc作为asio底层的事件调度器，接收事件
	void Start();
private:
	tcp::acceptor _acceptor;
	net::io_context& _ioc;
//	tcp::socket _socket;//可以复用，接收完新的连接后继续接收新的连接

};

