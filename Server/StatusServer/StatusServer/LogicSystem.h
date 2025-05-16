#pragma once
#include "const.h"
class HttpConnection;
typedef std::function<void(std::shared_ptr<HttpConnection>)> HttpHandler;
 
class LogicSystem:public Singleton<LogicSystem>
{
	friend class Singleton<LogicSystem>;
public:
	~LogicSystem(){};
	bool HandleGet(std::string,std::shared_ptr<HttpConnection>);
	void RegGet(std::string, HttpHandler handler);//ע��·�ɺͶ�Ӧ�Ļص�������get_handlers
	void RegPost(std::string, HttpHandler handler);
	bool  HandlePost(std::string, std::shared_ptr<HttpConnection>);
private:
	LogicSystem();
	//_post_handlers��_get_handlers�ֱ���post�����get����Ļص�����map��keyΪ·�ɣ�valueΪ�ص�����
	std::map<std::string, HttpHandler> _post_handlers;
	std::map<std::string, HttpHandler> _get_handlers;

};

