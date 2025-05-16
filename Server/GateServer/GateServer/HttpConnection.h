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
	void CheckDeadline();//���socket�����Ƿ�ʱ���Ƿ��ڸ���ʱ������������շ���
	void WriteResponse();
	void HandleReq();
	void PreParseGetParam();
	tcp::socket _socket;
	beast::flat_buffer _buffer{ 8192 };//�����������ݣ���󳤶���8192Byte
	http::request<http::dynamic_body> _request;//���ڽ�������
	http::response<http::dynamic_body> _response;//������Ӧ�ͻ���
	net::steady_timer deadline_{
		_socket.get_executor(),std::chrono::seconds(60)
	};//���õײ�Ķ�ʱ���ĵ���������socket��������ʱ60��
	std::string _get_url;
	std::unordered_map<std::string, std::string> _get_params;
};

