#pragma once
#include "const.h"
#include "MysqlDao.h"


class MysqlMgr:public Singleton<MysqlMgr>
{
public:
	friend class  Singleton<MysqlMgr>;
	~MysqlMgr();
	int RegUser(const std::string& name, const std::string& email, const std::string& pwd);
	bool CheckEmail(const std::string& name, const std::string& email);
	bool UpdatePwd(const std::string& name, const std::string& pwd);
	bool CheckPwd(const std::string& email, const std::string& pwd, UserInfo& userInfo);
	//bool TestProcedure(const std::string& email, int& uid, string& name);
private:
	MysqlDao _Dao;
	MysqlMgr();





};

