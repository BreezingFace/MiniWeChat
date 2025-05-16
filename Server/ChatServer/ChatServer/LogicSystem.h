#pragma once
#include "const.h"
#include "CSession.h"
#include "StatusGrpcClient.h"
#include "MysqlDao.h"
#include "MysqlMgr.h"
typedef  function<void(shared_ptr<CSession>, const short& msg_id, const string& msg_data)> FunCallBack;
class LogicSystem:public Singleton<LogicSystem>
{
	friend class Singleton<LogicSystem>;//将Singleton<LogicSystem>设置为友元类，这样它就能访问LogicSystem的私有成员
private:
	LogicSystem();
	void RegisterCallBacks();//注册回调函数
	void DealMsg();
	void LoginHandler(shared_ptr<CSession> session, const short& msg_id, const string& msg_data);
	void SearchInfo(shared_ptr<CSession> session, const short& msg_id, const string& msg_data);
	void AddFriendApply(shared_ptr<CSession> session, const short& msg_id, const string& msg_data);
	void AuthFriendApply(std::shared_ptr<CSession> session, const short& msg_id, const string& msg_data);
	void DealChatTextMsg(std::shared_ptr<CSession> session, const short& msg_id, const string& msg_data);
	bool GetBaseInfo(std::string base_key, int uid, std::shared_ptr<UserInfo>& userinfo);
	bool isPureDigit(const std::string& str);
	void GetUserByUid(std::string uid_str, Json::Value& rtvalue);
	void GetUserByName(std::string name, Json::Value& rtvalue);
	bool GetFriendApplyInfo(int to_uid, std::vector<std::shared_ptr<ApplyInfo>>& list);
	bool GetFriendList(int self_id, std::vector<std::shared_ptr<UserInfo>>& user_list);
	
	std::queue<shared_ptr<LogicNode>> _msg_que;//队列能保证发送消息的有序性，并支持解耦
	std::mutex _mutex;
	std::condition_variable _consume;//条件变量
	std::thread _worker_thread;//逻辑线程，从逻辑队列取数据
	bool _b_stop;//逻辑控制
	std::map<short, FunCallBack> _fun_callback;//通过map,将msgid与相应的回调函数绑定

public:
	~LogicSystem();//设置为公有，是为了让智能指针自动析构
	void PostMsgToQue(shared_ptr<LogicNode> msg);


};

