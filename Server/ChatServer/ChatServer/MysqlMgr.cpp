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
bool MysqlMgr::AddFriendApply(const int& from, const int& to)
{
	return _Dao.AddFriendApply(from,to);
}
bool MysqlMgr::AuthFriendApply(const int& from, const int& to)
{
	return _Dao.AuthFriendApply(from, to);
}
bool MysqlMgr::AddFriend(const int& from, const int& to, std::string back_name)
{
	return _Dao.AddFriend(from , to , back_name);
}
bool MysqlMgr::GetFriendList(int self_id, std::vector<std::shared_ptr<UserInfo>>& user_info)
{
	return _Dao.GetFriendList(self_id, user_info);
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
std::shared_ptr<UserInfo> MysqlMgr::GetUser(int uid)
{
	return _Dao.GetUser(uid);
}

std::shared_ptr<UserInfo> MysqlMgr::GetUser(std::string name)
{
	return _Dao.GetUser(name);
}

bool MysqlMgr::GetApplyList(int touid, std::vector<std::shared_ptr<ApplyInfo>>& applyList, int begin, int limit)
{
	return _Dao.GetApplyList(touid, applyList, begin, limit);
}
