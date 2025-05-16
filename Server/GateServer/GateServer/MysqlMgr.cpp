#include "MysqlMgr.h"
MysqlMgr::~MysqlMgr() {

}
MysqlMgr::MysqlMgr() {

}

int MysqlMgr::RegUser(const std::string& name, const std::string& email, const std::string& pwd) {
	return this->_Dao.RegUser(name, email, pwd);
}

bool MysqlMgr::CheckEmail(const std::string& name, const std::string& email)
{
	return this->_Dao.CheckEmail(name, email);
}

bool MysqlMgr::UpdatePwd(const std::string& name, const std::string& pwd)
{
	return this->_Dao.UpdatePwd(name,pwd);
}



bool MysqlMgr::CheckPwd(const std::string& email, const std::string& pwd, UserInfo& userInfo) {
	return this->_Dao.CheckPwd(email, pwd, userInfo);
}
//
//bool MysqlMgr::TestProcedure(const std::string& email, int& uid, string& name) {
//	return _dao.TestProcedure(email, uid, name);
//}
//
//
//bool MysqlMgr::CheckEmail(const std::string& name, const std::string& email) {
//	return _dao.CheckEmail(name, email);
//}
