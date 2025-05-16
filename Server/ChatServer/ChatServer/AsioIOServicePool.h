#pragma once
#include <vector>
#include <boost/asio.hpp>
#include "Singleton.h"
class AsioIOServicePool:public Singleton<AsioIOServicePool>
{
	friend Singleton<AsioIOServicePool>;
public:
	using IOService = boost::asio::io_context;
	using Work = boost::asio::io_context::work;//work机制保证了当调用了io_context.run()后若事件队列中没有事件，io_context不会退出，而是阻塞等待事件的到来
	using WorkPtr = std::unique_ptr<Work>;
	~AsioIOServicePool();
	AsioIOServicePool(const AsioIOServicePool&) = delete;
	AsioIOServicePool& operator=(const AsioIOServicePool&) = delete;
	// 使用 round-robin 的方式返回一个 io_service
	boost::asio::io_context& GetIOService();
	//停止使用线程池
	void Stop();
private:
	AsioIOServicePool(std::size_t size = 2/*std::thread::hardware_concurrency()*/);//使用2个线程，后面的函数返回的是CPU核数
	std::vector<IOService> _ioServices;
	std::vector<WorkPtr> _works;//每个io_context对应一个WorkPtr
	std::vector<std::thread> _threads;
	std::size_t   _nextIOService;
};

