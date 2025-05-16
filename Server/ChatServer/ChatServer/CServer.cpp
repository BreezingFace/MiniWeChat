#include "CServer.h"
//#include "HttpConnection.h"
#include "AsioIOServicePool.h"
#include  "UserMgr.h"

CServer::CServer(boost::asio::io_context& ioc, unsigned short port) :_ioc(ioc),_port(port),
_acceptor(ioc, tcp::endpoint(tcp::v4(), port))//tcp::v4()接受本地服务器的任何一个ipv4地址
{
	cout << "Server start success, listen on port : " << _port << endl;
	StartAccept();
}


CServer::~CServer() {
	cout << "Server destruct listen on port : " << _port << endl;
}

void CServer::HandleAccept(shared_ptr<CSession> new_session, const boost::system::error_code& error) {
	if (!error) {
		new_session->Start();//启动会话的异步读写
		lock_guard<mutex> lock(_mutex);///加锁保护 _sessions 容器，插入新会话。
		_sessions.insert(make_pair(new_session->GetSessionId(), new_session));
	}
	else {
		cout << "session accept failed, error is " << error.what() << endl;
	}

	StartAccept();
}

void CServer::StartAccept() {
	auto& io_context = AsioIOServicePool::GetInstance()->GetIOService();
	shared_ptr<CSession> new_session = make_shared<CSession>(io_context, this);
	//async_accept 是非阻塞的，通过回调函数 HandleAccept 通知结果。placeholders::_1：占位符，表示 async_accept 的第一个参数（error_code）会传递给 HandleAccept。
	_acceptor.async_accept(new_session->GetSocket(), std::bind(&CServer::HandleAccept, this, new_session, placeholders::_1));
}

void CServer::ClearSession(std::string session_id) {

	if (_sessions.find(session_id) != _sessions.end()) {
		////移除用户和session的关联
		UserMgr::GetInstance()->RmvUserSession(_sessions[session_id]->GetUserId());
	}

	
	lock_guard<mutex> lock(_mutex);
	_sessions.erase(session_id);
	

}
