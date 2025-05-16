#pragma once
#include "const.h"
#include <thread>
#include <jdbc/mysql_driver.h>
#include <jdbc/mysql_connection.h>
#include <jdbc/cppconn/prepared_statement.h>
#include <jdbc/cppconn/statement.h>
#include <jdbc/cppconn/exception.h>
#include <jdbc/cppconn/resultset.h>
#include "data.h"
// SqlConnection 类：封装一个 MySQL 连接对象及其最后一次操作的时间戳
class SqlConnection {
public:
	// 构造函数：初始化 MySQL 连接对象和最后一次操作的时间戳
	SqlConnection(sql::Connection* con, int64_t lasttime) : _con(con), _last_oper_time(lasttime) {}

	std::unique_ptr<sql::Connection> _con;  // MySQL 连接对象（使用智能指针管理）
	int64_t _last_oper_time;                // 最后一次操作的时间戳
};
// MySqlPool 类：MySQL 数据库连接池
class MySqlPool {
public:
    // 构造函数：初始化连接池
    MySqlPool(const std::string& url, const std::string& user, const std::string& pass, const std::string& schema, int poolSize)
        : url_(url), user_(user), pass_(pass), schema_(schema), poolSize_(poolSize), b_stop_(false) {
        try {
            // 初始化连接池，创建指定数量的连接
            for (int i = 0; i < poolSize_; ++i) {
                sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();  // 获取 MySQL 驱动实例
                auto* con = driver->connect(url_, user_, pass_);  // 创建连接
                con->setSchema(schema_);  // 设置数据库模式

                // 获取当前时间戳
                auto currentTime = std::chrono::system_clock::now().time_since_epoch();

                long long timestamp = std::chrono::duration_cast<std::chrono::seconds>(currentTime).count();

                // 将连接对象和时间戳封装为 SqlConnection，并存入连接池
                pool_.push(std::make_unique<SqlConnection>(con, timestamp));
            }

            // 启动后台线程，定期检查连接的健康状态
            _check_thread = std::thread([this]() {
                while (!b_stop_) {
                    checkConnection();  // 检查连接
                    std::this_thread::sleep_for(std::chrono::seconds(60));  // 每隔 60 秒检查一次
                }
                });

            _check_thread.detach();  // 分离线程，使其在后台运行
        }
        catch (sql::SQLException& e) {
            // 处理异常：打印错误信息
            std::cout << "mysql pool init failed, error is " << e.what() << std::endl;
        }
    }

    // 检查连接池中的连接是否有效
    void checkConnection() {
        std::lock_guard<std::mutex> guard(mutex_);  // 加锁，确保线程安全
        int poolsize = pool_.size();  // 获取当前连接池大小

        // 获取当前时间戳
        auto currentTime = std::chrono::system_clock::now().time_since_epoch();
        long long timestamp = std::chrono::duration_cast<std::chrono::seconds>(currentTime).count();

        // 遍历连接池中的每个连接
        for (int i = 0; i < poolsize; i++) {
            auto con = std::move(pool_.front());  // 取出连接
            pool_.pop();

            // 使用 Defer 确保连接在检查完成后被放回连接池
            Defer defer([this, &con]() {
                pool_.push(std::move(con));
                });

            // 如果连接最近被使用过，跳过检查
            if (timestamp - con->_last_oper_time < 5) {
                continue;
            }

            // 检查连接是否有效
            try {
                std::unique_ptr<sql::Statement> stmt(con->_con->createStatement());  // 创建 Statement 对象
                stmt->executeQuery("SELECT 1");  // 执行简单查询
                con->_last_oper_time = timestamp;  // 更新最后一次操作的时间戳
            }
            catch (sql::SQLException& e) {
                // 如果连接失效，重新创建连接
                std::cout << "Error keeping connection alive: " << e.what() << std::endl;
                sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
                auto* newcon = driver->connect(url_, user_, pass_);  // 创建新连接
                newcon->setSchema(schema_);
                con->_con.reset(newcon);  // 替换旧的连接
                con->_last_oper_time = timestamp;  // 更新最后一次操作的时间戳
            }
        }
    }

    // 从连接池中获取一个连接
    std::unique_ptr<SqlConnection> getConnection() {
        std::unique_lock<std::mutex> lock(mutex_);  // 加锁，确保线程安全

        // 等待直到连接池中有可用连接或连接池关闭
        cond_.wait(lock, [this] {
            if (b_stop_) {
                return true;
            }
            return !pool_.empty();
            });

        // 如果连接池已关闭，返回空指针
        if (b_stop_) {
            return nullptr;
        }

        // 取出一个连接并返回
        std::unique_ptr<SqlConnection> con(std::move(pool_.front()));
        pool_.pop();
        return con;
    }

    // 将连接返回到连接池中
    void returnConnection(std::unique_ptr<SqlConnection> con) {
        std::unique_lock<std::mutex> lock(mutex_);  // 加锁，确保线程安全

        // 如果连接池已关闭，直接返回
        if (b_stop_) {
            return;
        }

        // 将连接放回连接池，并通知等待的线程
        pool_.push(std::move(con));
        cond_.notify_one();
    }

    // 关闭连接池
    void Close() {
        b_stop_ = true;  // 设置关闭标志
        cond_.notify_all();  // 通知所有等待的线程
    }

    // 析构函数：释放连接池中的所有连接
    ~MySqlPool() {
        std::unique_lock<std::mutex> lock(mutex_);  // 加锁，确保线程安全
        while (!pool_.empty()) {
            pool_.pop();  // 清空连接池
        }
    }

private:
    std::string url_;  // 数据库连接 URL
    std::string user_;  // 数据库用户名
    std::string pass_;  // 数据库密码
    std::string schema_;  // 数据库名称
    int poolSize_;  // 连接池大小
    std::queue<std::unique_ptr<SqlConnection>> pool_;  // 连接池（使用队列存储连接）
    std::mutex mutex_;  // 互斥锁，用于线程安全
    std::condition_variable cond_;  // 条件变量，用于线程同步
    std::atomic<bool> b_stop_;  // 标志位，表示连接池是否已关闭
    std::thread _check_thread;  // 后台线程，用于定期检查连接
};

//// UserInfo 结构体：存储用户信息
//struct UserInfo {
//    std::string name;  // 用户名
//    std::string pwd;   // 密码
//    int uid;           // 用户 ID
//    std::string email; // 邮箱
//};

// MysqlDao 类：数据库访问对象，封装了对 MySQL 数据库的操作
class MysqlDao {
public:
    MysqlDao();  // 构造函数
    ~MysqlDao(); // 析构函数

    // 注册用户
    int RegUser(const std::string& name, const std::string& email, const std::string& pwd);

    //// 使用事务注册用户
    //int RegUserTransaction(const std::string& name, const std::string& email, const std::string& pwd, const std::string& icon);

    //// 检查邮箱是否已注册
    bool CheckEmail(const std::string& name, const std::string& email);

    //// 更新用户密码
    bool UpdatePwd(const std::string& name, const std::string& newpwd);

    //// 检查用户密码是否正确
    bool CheckPwd(const std::string& name, const std::string& pwd, UserInfo& userInfo);

    //// 测试存储过程
    //bool TestProcedure(const std::string& email, int& uid, std::string& name);

    std::shared_ptr<UserInfo> GetUser(int uid);

private:
    std::unique_ptr<MySqlPool> pool_;  // MySQL 连接池
};
