#pragma once
#include "const.h"
#include "CSession.h"
#include "StatusGrpcClient.h"
#include "MysqlDao.h"
#include "MysqlMgr.h"
typedef  function<void(shared_ptr<CSession>, const short& msg_id, const string& msg_data)> FunCallBack;
class LogicSystem:public Singleton<LogicSystem>
{
	friend class Singleton<LogicSystem>;//��Singleton<LogicSystem>����Ϊ��Ԫ�࣬���������ܷ���LogicSystem��˽�г�Ա
private:
	LogicSystem();
	void RegisterCallBacks();//ע��ص�����
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
	
	std::queue<shared_ptr<LogicNode>> _msg_que;//�����ܱ�֤������Ϣ�������ԣ���֧�ֽ���
	std::mutex _mutex;
	std::condition_variable _consume;//��������
	std::thread _worker_thread;//�߼��̣߳����߼�����ȡ����
	bool _b_stop;//�߼�����
	std::map<short, FunCallBack> _fun_callback;//ͨ��map,��msgid����Ӧ�Ļص�������

public:
	~LogicSystem();//����Ϊ���У���Ϊ��������ָ���Զ�����
	void PostMsgToQue(shared_ptr<LogicNode> msg);


};

