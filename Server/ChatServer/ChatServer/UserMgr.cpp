#include "UserMgr.h"
#include "CSession.h"
#include "RedisMgr.h"

UserMgr::~UserMgr()
{
	_uid_to_session.clear();
}

std::shared_ptr<CSession> UserMgr::GetSession(int uid)
{    
	std::lock_guard<std::mutex> lock(_session_mtx);
	if (_uid_to_session.find(uid) != _uid_to_session.end()) {
		return _uid_to_session[uid];
	}
	return nullptr;
}

void UserMgr::SetUserSession(int uid, std::shared_ptr<CSession> session)
{
	std::lock_guard<std::mutex> lock(_session_mtx);
	_uid_to_session[uid] = session;
}

void UserMgr::RmvUserSession(int uid)
{
	std::lock_guard<std::mutex> lock(_session_mtx);
	if (_uid_to_session.find(uid) != _uid_to_session.end()) {
		_uid_to_session.erase(uid);
	}
}

UserMgr::UserMgr()
{
}
