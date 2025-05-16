#pragma once
#include "const.h"
#include "CSession.h"
class CServer:public std::enable_shared_from_this<CServer>
{
public:
	CServer(boost::asio::io_context& ioc, unsigned short port);//ioc作为asio底层的事件调度器，接收事件
	~CServer();
	void ClearSession(std::string);
	
private:
	void StartAccept();
	void HandleAccept(std::shared_ptr<CSession>, const boost::system::error_code& ec);
	unsigned short _port;
	tcp::acceptor _acceptor;
	net::io_context& _ioc;
	std::map<std::string, std::shared_ptr<CSession>> _sessions;
	std::mutex _mutex;
//	tcp::socket _socket;//可以复用，接收完新的连接后继续接收新的连接

};

