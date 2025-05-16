#include "MysqlDao.h"
#include "ConfigMgr.h"

MysqlDao::MysqlDao()
{
	auto& cfg = ConfigMgr::Inst();
	const auto& host = cfg["Mysql"]["Host"];
	const auto& port = cfg["Mysql"]["Port"];
	const auto& pwd = cfg["Mysql"]["Passwd"];
	const auto& schema = cfg["Mysql"]["Schema"];
	const auto& user = cfg["Mysql"]["User"];
	pool_.reset(new MySqlPool(host + ":" + port, user, pwd, schema, 5));
}

MysqlDao::~MysqlDao() {
	pool_->Close();
}

int MysqlDao::RegUser(const std::string& name, const std::string& email, const std::string& pwd)
{
	auto con = pool_->getConnection();
	try {
		if (con == nullptr) {
			return false;
		}
		// 准备调用存储过程
		/*
		创建一个预编译的 SQL 语句对象，用于调用存储过程 reg_user。

		CALL reg_user(?,?,?,@result) 是调用存储过程的 SQL 语句，其中 ? 是占位符，表示输入参数。

		@result 是会话变量，用于存储存储过程的输出结果。
		
		*/

		
		unique_ptr < sql::PreparedStatement > stmt(con->_con->prepareStatement("CALL reg_user(?,?,?,@result)"));
		// 设置输入参数
		stmt->setString(1, name);//1对应第一个?占位符
		stmt->setString(2, email);
		stmt->setString(3, pwd);

		// 由于PreparedStatement不直接支持注册输出参数，我们需要使用会话变量或其他方法来获取输出参数的值

		  // 执行存储过程
		/*
		执行存储过程 reg_user。
		存储过程会根据输入参数执行注册逻辑，并将结果存储在会话变量 @result 中。
		*/
		stmt->execute();
		// 如果存储过程设置了会话变量或有其他方式获取输出参数的值，你可以在这里执行SELECT查询来获取它们
	   // 例如，如果存储过程设置了一个会话变量@result来存储输出结果，可以这样获取：
		/*
		createStatement：
		创建一个新的 SQL 语句对象，用于查询会话变量 @result。
		executeQuery：
		执行查询语句 SELECT @result AS result，获取存储过程的输出结果。
		unique_ptr<sql::ResultSet>：
		存储查询结果。
		*/
		unique_ptr<sql::Statement> stmtResult(con->_con->createStatement());
		/*
		SELECT @result AS result 是必要的，因为 MySQL Connector/C++ 不支持直接获取存储过程的输出参数。通过查询会话变量 @result，可以间接获取存储过程的输出结果。
		*/
		unique_ptr<sql::ResultSet> res(stmtResult->executeQuery("SELECT @result AS result"));
		/*
		res->next()：
		移动到查询结果的第一行。
		getInt("result")：
		获取 result 列的值（即存储过程的输出结果）。
		cout：
		打印结果到控制台（用于调试）。
		pool_->returnConnection：
		将连接返回到连接池中。
		return result：
		返回存储过程的输出结果。

		*/
		if (res->next()) {
			int result = res->getInt("result");
			cout << "Result: " << result << endl;
			pool_->returnConnection(std::move(con));
			return result;
		}
		pool_->returnConnection(std::move(con));
		return -1;
	}
	catch (sql::SQLException& e) {
		pool_->returnConnection(std::move(con));
		std::cerr << "SQLException: " << e.what();
		std::cerr << " (MySQL error code: " << e.getErrorCode();
		std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
		return -1;
	}
}

//int MysqlDao::RegUserTransaction(const std::string& name, const std::string& email, const std::string& pwd,
//	const std::string& icon)
//{
//	auto con = pool_->getConnection();
//	if (con == nullptr) {
//		return false;
//	}
//
//	Defer defer([this, &con] {
//		pool_->returnConnection(std::move(con));
//		});
//
//	try {
//		//开始事务
//		con->_con->setAutoCommit(false);
//		//执行第一个数据库操作，根据email查找用户
//			// 准备查询语句
//
//		std::unique_ptr<sql::PreparedStatement> pstmt_email(con->_con->prepareStatement("SELECT 1 FROM user WHERE email = ?"));
//
//		// 绑定参数
//		pstmt_email->setString(1, email);
//
//		// 执行查询
//		std::unique_ptr<sql::ResultSet> res_email(pstmt_email->executeQuery());
//
//		auto email_exist = res_email->next();
//		if (email_exist) {
//			con->_con->rollback();
//			std::cout << "email " << email << " exist";
//			return 0;
//		}
//
//		// 准备查询用户名是否重复
//		std::unique_ptr<sql::PreparedStatement> pstmt_name(con->_con->prepareStatement("SELECT 1 FROM user WHERE name = ?"));
//
//		// 绑定参数
//		pstmt_name->setString(1, name);
//
//		// 执行查询
//		std::unique_ptr<sql::ResultSet> res_name(pstmt_name->executeQuery());
//
//		auto name_exist = res_name->next();
//		if (name_exist) {
//			con->_con->rollback();
//			std::cout << "name " << name << " exist";
//			return 0;
//		}
//
//		// 准备更新用户id
//		std::unique_ptr<sql::PreparedStatement> pstmt_upid(con->_con->prepareStatement("UPDATE user_id SET id = id + 1"));
//
//		// 执行更新
//		pstmt_upid->executeUpdate();
//
//		// 获取更新后的 id 值
//		std::unique_ptr<sql::PreparedStatement> pstmt_uid(con->_con->prepareStatement("SELECT id FROM user_id"));
//		std::unique_ptr<sql::ResultSet> res_uid(pstmt_uid->executeQuery());
//		int newId = 0;
//		// 处理结果集
//		if (res_uid->next()) {
//			newId = res_uid->getInt("id");
//		}
//		else {
//			std::cout << "select id from user_id failed" << std::endl;
//			con->_con->rollback();
//			return -1;
//		}
//
//		// 插入user信息
//		std::unique_ptr<sql::PreparedStatement> pstmt_insert(con->_con->prepareStatement("INSERT INTO user (uid, name, email, pwd, nick, icon) "
//			"VALUES (?, ?, ?, ?,?,?)"));
//		pstmt_insert->setInt(1, newId);
//		pstmt_insert->setString(2, name);
//		pstmt_insert->setString(3, email);
//		pstmt_insert->setString(4, pwd);
//		pstmt_insert->setString(5, name);
//		pstmt_insert->setString(6, icon);
//		//执行插入
//		pstmt_insert->executeUpdate();
//		// 提交事务
//		con->_con->commit();
//		std::cout << "newuser insert into user success" << std::endl;
//		return newId;
//	}
//	catch (sql::SQLException& e) {
//		// 如果发生错误，回滚事务
//		if (con) {
//			con->_con->rollback();
//		}
//		std::cerr << "SQLException: " << e.what();
//		std::cerr << " (MySQL error code: " << e.getErrorCode();
//		std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
//		return -1;
//	}
//}
//
bool MysqlDao::CheckEmail(const std::string& name, const std::string& email) {
	auto con = pool_->getConnection();
	try {
		if (con == nullptr) {
			return false;
		}

		// 准备查询语句
		std::unique_ptr<sql::PreparedStatement> pstmt(con->_con->prepareStatement("SELECT email FROM user WHERE name = ?"));

		// 绑定参数
		pstmt->setString(1, name);

		// 执行查询
		std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());

		// 遍历结果集
		while (res->next()) {
			std::cout << "Check Email: " << res->getString("email") << std::endl;
			if (email != res->getString("email")) {
				pool_->returnConnection(std::move(con));
				return false;
			}
			pool_->returnConnection(std::move(con));
			return true;
		}
		return false;
	}
	catch (sql::SQLException& e) {
		pool_->returnConnection(std::move(con));
		std::cerr << "SQLException: " << e.what();
		std::cerr << " (MySQL error code: " << e.getErrorCode();
		std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
		return false;
	}
}

bool MysqlDao::UpdatePwd(const std::string& name, const std::string& newpwd) {
	auto con = pool_->getConnection();
	try {
		if (con == nullptr) {
			return false;
		}

		// 准备查询语句
		std::unique_ptr<sql::PreparedStatement> pstmt(con->_con->prepareStatement("UPDATE user SET pwd = ? WHERE name = ?"));

		// 绑定参数
		pstmt->setString(2, name);
		pstmt->setString(1, newpwd);

		// 执行更新
		int updateCount = pstmt->executeUpdate();

		std::cout << "Updated rows: " << updateCount << std::endl;
		pool_->returnConnection(std::move(con));
		return true;
	}
	catch (sql::SQLException& e) {
		pool_->returnConnection(std::move(con));
		std::cerr << "SQLException: " << e.what();
		std::cerr << " (MySQL error code: " << e.getErrorCode();
		std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
		return false;
	}
}

bool MysqlDao::CheckPwd(const std::string& email, const std::string& pwd, UserInfo& userInfo) {
	auto con = pool_->getConnection();
	if (con == nullptr) {
		return false;
	}

	Defer defer([this, &con]() {
		pool_->returnConnection(std::move(con));
		});

	try {
		// 准备SQL语句
		std::unique_ptr<sql::PreparedStatement> pstmt(con->_con->prepareStatement("SELECT * FROM user WHERE email = ?"));
		pstmt->setString(1, email); // 将username替换为你要查询的用户名

		// 执行查询
		std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
		std::string origin_pwd = "";
		// 遍历结果集
		while (res->next()) {
			origin_pwd = res->getString("pwd");
			// 输出查询到的密码
			std::cout << "Password: " << origin_pwd << std::endl;
			break;
		}

		if (pwd != origin_pwd) {
			return false;
		}
		userInfo.name = res->getString("name");
		userInfo.email = res->getString("email");
		userInfo.uid = res->getInt("uid");
		userInfo.pwd = origin_pwd;
		return true;
	}
	catch (sql::SQLException& e) {
		std::cerr << "SQLException: " << e.what();
		std::cerr << " (MySQL error code: " << e.getErrorCode();
		std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
		return false;
	}
}

bool MysqlDao::AddFriendApply(const int& from, const int& to)
{
	auto con = pool_->getConnection();
	if (con == nullptr) {
		return false;
	}

	Defer defer([this, &con]() {
		pool_->returnConnection(std::move(con));
		});

	try {
		// 准备SQL语句
		std::unique_ptr<sql::PreparedStatement> pstmt(con->_con->prepareStatement("INSERT INTO friend_apply (from_uid, to_uid) values (?,?) "
			"ON DUPLICATE KEY UPDATE from_uid = from_uid, to_uid = to_uid "));
		//ON DUPLICATE KEY UPDATE：
		//作用：如果(from_uid, to_uid) 组合已存在（违反唯一约束），则执行更新而非报错
	    //当前行为：更新时实际不修改任何值（from_uid = from_uid 相当于无操作）
		pstmt->setInt(1, from);
		pstmt->setInt(2, to);
		// 执行更新
		int rowAffected = pstmt->executeUpdate();
		if (rowAffected < 0) {
			return false;
		}
		return true;
	}
	catch (sql::SQLException& e) {
		std::cerr << "SQLException: " << e.what();
		std::cerr << " (MySQL error code: " << e.getErrorCode();
		std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
		return false;
	}


	return true;
}

bool MysqlDao::AuthFriendApply(const int& from, const int& to)
{
	auto con = pool_->getConnection();
	if (con == nullptr) {
		return false;
	}

	Defer defer([this, &con]() {
		pool_->returnConnection(std::move(con));
		});

	try {
		// 准备SQL语句
		std::unique_ptr<sql::PreparedStatement> pstmt(con->_con->prepareStatement("UPDATE friend_apply SET status = 1 "
			"WHERE from_uid = ? AND to_uid = ?"));
		//反过来的申请时from，验证时to
		pstmt->setInt(1, to); // from id
		pstmt->setInt(2, from);
		// 执行更新
		int rowAffected = pstmt->executeUpdate();
		if (rowAffected < 0) {
			return false;
		}
		return true;
	}
	catch (sql::SQLException& e) {
		std::cerr << "SQLException: " << e.what();
		std::cerr << " (MySQL error code: " << e.getErrorCode();
		std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
		return false;
	}


	return true;
}

bool MysqlDao::AddFriend(const int& from, const int& to, std::string back_name)
{
	auto con = pool_->getConnection();
	if (con == nullptr) {
		return false;
	}

	Defer defer([this, &con]() {
		pool_->returnConnection(std::move(con));
		});

	try {

		//开始事务
		con->_con->setAutoCommit(false);

		// 准备第一个SQL语句, 插入认证方好友数据
		std::unique_ptr<sql::PreparedStatement> pstmt(con->_con->prepareStatement("INSERT IGNORE INTO friend(self_id, friend_id, back) "
			"VALUES (?, ?, ?) "
		));
		//反过来的申请时from，验证时to
		pstmt->setInt(1, from); // from id
		pstmt->setInt(2, to);
		pstmt->setString(3, back_name);
		// 执行更新
		int rowAffected = pstmt->executeUpdate();
		if (rowAffected < 0) {
			con->_con->rollback();
			return false;
		}

		//准备第二个SQL语句，插入申请方好友数据
		std::unique_ptr<sql::PreparedStatement> pstmt2(con->_con->prepareStatement("INSERT IGNORE INTO friend(self_id, friend_id, back) "
			"VALUES (?, ?, ?) "
		));
		//反过来的申请时from，验证时to
		pstmt2->setInt(1, to); // from id
		pstmt2->setInt(2, from);
		pstmt2->setString(3, "");
		// 执行更新
		int rowAffected2 = pstmt2->executeUpdate();
		if (rowAffected2 < 0) {
			con->_con->rollback();
			return false;
		}

		// 提交事务
		con->_con->commit();
		std::cout << "addfriend insert friends success" << std::endl;

		return true;
	}
	catch (sql::SQLException& e) {
		// 如果发生错误，回滚事务
		if (con) {
			con->_con->rollback();
		}
		std::cerr << "SQLException: " << e.what();
		std::cerr << " (MySQL error code: " << e.getErrorCode();
		std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
		return false;
	}


	return true;
}

bool MysqlDao::GetApplyList(int touid, std::vector<std::shared_ptr<ApplyInfo>>& applyList, int offset, int limit)
{
	auto con = pool_->getConnection();
	if (con == nullptr) {
		return false;
	}

	Defer defer([this, &con]() {
		pool_->returnConnection(std::move(con));
		});


	try {
		// 准备SQL语句, 根据起始id和限制条数返回列表
		std::unique_ptr<sql::PreparedStatement> pstmt(con->_con->prepareStatement("select apply.from_uid, apply.status, user.name, "
			"user.nick, user.sex from friend_apply as apply join user on apply.from_uid = user.uid where apply.to_uid = ? "
			"and apply.id > ? order by apply.id ASC LIMIT ? "));

		pstmt->setInt(1, touid); // 将uid替换为你要查询的uid
		pstmt->setInt(2, offset); // 起始id
		pstmt->setInt(3, limit); //偏移量
		// 执行查询
		std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
		// 遍历结果集
		while (res->next()) {
			auto name = res->getString("name");
			auto uid = res->getInt("from_uid");
			auto status = res->getInt("status");
			auto nick = res->getString("nick");
			auto sex = res->getInt("sex");
			auto apply_ptr = std::make_shared<ApplyInfo>(uid, name, "", "", nick, sex, status);
			applyList.push_back(apply_ptr);
		}
		return true;
	}
	catch (sql::SQLException& e) {
		std::cerr << "SQLException: " << e.what();
		std::cerr << " (MySQL error code: " << e.getErrorCode();
		std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
		return false;
	}
}

std::shared_ptr<UserInfo> MysqlDao::GetUser(int uid)
{
	
		auto con = pool_->getConnection();
		if (con == nullptr) {
			return nullptr;
		}

		Defer defer([this, &con]() {
			pool_->returnConnection(std::move(con));
			});

		try {
			// 准备SQL语句
			//创建预处理语句（PreparedStatement），数据库会解析并优化 SQL。
			std::unique_ptr<sql::PreparedStatement> pstmt(con->_con->prepareStatement("SELECT * FROM user WHERE uid = ?"));
			pstmt->setInt(1, uid); // 动态绑定 uid 的值，类型安全（setInt 确保传入的是整数）。

			// 执行参数化查询
			/*
			优点：
			1.彻底防御SQL注入攻击。参数化查询将 SQL逻辑 和 数据值 完全分离。SQL语句模板预先编译，用户输入的数据始终被视为纯数据（而非代码）。
			2.性能优化（数据库引擎级加速）。数据库首次执行时会解析SQL模板并生成执行计划，后续只需传递参数值，跳过重复的语法分析和优化阶段。
			3.数据类型安全校验。通过 setInt()、setString() 等方法绑定参数时，数据库驱动会强制检查类型匹配，避免隐式转换错误。
			*/
			std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
			std::shared_ptr<UserInfo> user_ptr = nullptr;
			// 返回结果集 ResultSet，通过 res->next() 遍历行。
			while (res->next()) {
				user_ptr.reset(new UserInfo);
				user_ptr->pwd = res->getString("pwd");
				user_ptr->email = res->getString("email");
				user_ptr->name = res->getString("name");
				user_ptr->nick = res->getString("nick");
				user_ptr->desc = res->getString("desc");
				user_ptr->sex = res->getInt("sex");
				user_ptr->icon = res->getString("icon");
				user_ptr->uid = uid;
				break;
			}
			return user_ptr;
		}
		catch (sql::SQLException& e) {
			std::cerr << "SQLException: " << e.what();
			std::cerr << " (MySQL error code: " << e.getErrorCode();
			std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
			return nullptr;
		}
	
}

std::shared_ptr<UserInfo> MysqlDao::GetUser(string name)
{
	auto con = pool_->getConnection();
	if (con == nullptr) {
		return nullptr;
	}

	Defer defer([this, &con]() {
		pool_->returnConnection(std::move(con));
		});

	try {
		// 准备SQL语句
		std::unique_ptr<sql::PreparedStatement> pstmt(con->_con->prepareStatement("SELECT * FROM user WHERE name = ?"));
		pstmt->setString(1, name); // 将uid替换为你要查询的uid

		// 执行查询
		std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
		std::shared_ptr<UserInfo> user_ptr = nullptr;
		// 遍历结果集
		while (res->next()) {
			user_ptr.reset(new UserInfo);
			user_ptr->pwd = res->getString("pwd");
			user_ptr->email = res->getString("email");
			user_ptr->name = res->getString("name");
			user_ptr->nick = res->getString("nick");
			user_ptr->desc = res->getString("desc");
			user_ptr->icon = res->getString("icon");
			user_ptr->sex = res->getInt("sex");
			user_ptr->uid = res->getInt("uid");
			break;
		}
		return user_ptr;
	}
	catch (sql::SQLException& e) {
		std::cerr << "SQLException: " << e.what();
		std::cerr << " (MySQL error code: " << e.getErrorCode();
		std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
		return nullptr;
	}


}

bool MysqlDao::GetFriendList(int self_id, std::vector<std::shared_ptr<UserInfo>>& user_info_list)
{
	auto con = pool_->getConnection();
	if (con == nullptr) {
		return false;
	}

	Defer defer([this, &con]() {
		pool_->returnConnection(std::move(con));
		});


	try {
		// 准备SQL语句, 根据起始id和限制条数返回列表
		std::unique_ptr<sql::PreparedStatement> pstmt(con->_con->prepareStatement("select * from friend where self_id = ? "));

		pstmt->setInt(1, self_id); // 将uid替换为你要查询的uid

		// 执行查询
		std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
		// 遍历结果集
		while (res->next()) {
			auto friend_id = res->getInt("friend_id");
			auto back = res->getString("back");
			//再一次查询friend_id对应的信息
			auto user_info = GetUser(friend_id);
			if (user_info == nullptr) {
				continue;
			}

			user_info->back = user_info->name;
			user_info_list.push_back(user_info);
		}
		return true;
	}
	catch (sql::SQLException& e) {
		std::cerr << "SQLException: " << e.what();
		std::cerr << " (MySQL error code: " << e.getErrorCode();
		std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
		return false;
	}

	return true;
}

//bool MysqlDao::TestProcedure(const std::string& email, int& uid, string& name) {
//	auto con = pool_->getConnection();
//	try {
//		if (con == nullptr) {
//			return false;
//		}
//
//		Defer defer([this, &con]() {
//			pool_->returnConnection(std::move(con));
//			});
//		// 准备调用存储过程
//		unique_ptr < sql::PreparedStatement > stmt(con->_con->prepareStatement("CALL test_procedure(?,@userId,@userName)"));
//		// 设置输入参数
//		stmt->setString(1, email);
//
//		// 由于PreparedStatement不直接支持注册输出参数，我们需要使用会话变量或其他方法来获取输出参数的值
//
//		  // 执行存储过程
//		stmt->execute();
//		// 如果存储过程设置了会话变量或有其他方式获取输出参数的值，你可以在这里执行SELECT查询来获取它们
//	   // 例如，如果存储过程设置了一个会话变量@result来存储输出结果，可以这样获取：
//		unique_ptr<sql::Statement> stmtResult(con->_con->createStatement());
//		unique_ptr<sql::ResultSet> res(stmtResult->executeQuery("SELECT @userId AS uid"));
//		if (!(res->next())) {
//			return false;
//		}
//
//		uid = res->getInt("uid");
//		cout << "uid: " << uid << endl;
//
//		stmtResult.reset(con->_con->createStatement());
//		res.reset(stmtResult->executeQuery("SELECT @userName AS name"));
//		if (!(res->next())) {
//			return false;
//		}
//
//		name = res->getString("name");
//		cout << "name: " << name << endl;
//		return true;
//
//	}
//	catch (sql::SQLException& e) {
//		std::cerr << "SQLException: " << e.what();
//		std::cerr << " (MySQL error code: " << e.getErrorCode();
//		std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
//		return false;
//	}
//}
