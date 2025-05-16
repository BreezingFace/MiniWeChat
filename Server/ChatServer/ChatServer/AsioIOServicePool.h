#pragma once
#include <vector>
#include <boost/asio.hpp>
#include "Singleton.h"
class AsioIOServicePool:public Singleton<AsioIOServicePool>
{
	friend Singleton<AsioIOServicePool>;
public:
	using IOService = boost::asio::io_context;
	using Work = boost::asio::io_context::work;//work���Ʊ�֤�˵�������io_context.run()�����¼�������û���¼���io_context�����˳������������ȴ��¼��ĵ���
	using WorkPtr = std::unique_ptr<Work>;
	~AsioIOServicePool();
	AsioIOServicePool(const AsioIOServicePool&) = delete;
	AsioIOServicePool& operator=(const AsioIOServicePool&) = delete;
	// ʹ�� round-robin �ķ�ʽ����һ�� io_service
	boost::asio::io_context& GetIOService();
	//ֹͣʹ���̳߳�
	void Stop();
private:
	AsioIOServicePool(std::size_t size = 2/*std::thread::hardware_concurrency()*/);//ʹ��2���̣߳�����ĺ������ص���CPU����
	std::vector<IOService> _ioServices;
	std::vector<WorkPtr> _works;//ÿ��io_context��Ӧһ��WorkPtr
	std::vector<std::thread> _threads;
	std::size_t   _nextIOService;
};

