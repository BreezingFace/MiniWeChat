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
// SqlConnection �ࣺ��װһ�� MySQL ���Ӷ��������һ�β�����ʱ���
class SqlConnection {
public:
	// ���캯������ʼ�� MySQL ���Ӷ�������һ�β�����ʱ���
	SqlConnection(sql::Connection* con, int64_t lasttime) : _con(con), _last_oper_time(lasttime) {}

	std::unique_ptr<sql::Connection> _con;  // MySQL ���Ӷ���ʹ������ָ�����
	int64_t _last_oper_time;                // ���һ�β�����ʱ���
};
// MySqlPool �ࣺMySQL ���ݿ����ӳ�
class MySqlPool {
public:
    // ���캯������ʼ�����ӳ�
    MySqlPool(const std::string& url, const std::string& user, const std::string& pass, const std::string& schema, int poolSize)
        : url_(url), user_(user), pass_(pass), schema_(schema), poolSize_(poolSize), b_stop_(false) {
        try {
            // ��ʼ�����ӳأ�����ָ������������
            for (int i = 0; i < poolSize_; ++i) {
                sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();  // ��ȡ MySQL ����ʵ��
                auto* con = driver->connect(url_, user_, pass_);  // ��������
                con->setSchema(schema_);  // �������ݿ�ģʽ

                // ��ȡ��ǰʱ���
                auto currentTime = std::chrono::system_clock::now().time_since_epoch();

                long long timestamp = std::chrono::duration_cast<std::chrono::seconds>(currentTime).count();

                // �����Ӷ����ʱ�����װΪ SqlConnection�����������ӳ�
                pool_.push(std::make_unique<SqlConnection>(con, timestamp));
            }

            // ������̨�̣߳����ڼ�����ӵĽ���״̬
            _check_thread = std::thread([this]() {
                while (!b_stop_) {
                    checkConnection();  // �������
                    std::this_thread::sleep_for(std::chrono::seconds(60));  // ÿ�� 60 ����һ��
                }
                });

            _check_thread.detach();  // �����̣߳�ʹ���ں�̨����
        }
        catch (sql::SQLException& e) {
            // �����쳣����ӡ������Ϣ
            std::cout << "mysql pool init failed, error is " << e.what() << std::endl;
        }
    }

    // ������ӳ��е������Ƿ���Ч
    void checkConnection() {
        std::lock_guard<std::mutex> guard(mutex_);  // ������ȷ���̰߳�ȫ
        int poolsize = pool_.size();  // ��ȡ��ǰ���ӳش�С

        // ��ȡ��ǰʱ���
        auto currentTime = std::chrono::system_clock::now().time_since_epoch();
        long long timestamp = std::chrono::duration_cast<std::chrono::seconds>(currentTime).count();

        // �������ӳ��е�ÿ������
        for (int i = 0; i < poolsize; i++) {
            auto con = std::move(pool_.front());  // ȡ������
            pool_.pop();

            // ʹ�� Defer ȷ�������ڼ����ɺ󱻷Ż����ӳ�
            Defer defer([this, &con]() {
                pool_.push(std::move(con));
                });

            // ������������ʹ�ù����������
            if (timestamp - con->_last_oper_time < 5) {
                continue;
            }

            // ��������Ƿ���Ч
            try {
                std::unique_ptr<sql::Statement> stmt(con->_con->createStatement());  // ���� Statement ����
                stmt->executeQuery("SELECT 1");  // ִ�м򵥲�ѯ
                con->_last_oper_time = timestamp;  // �������һ�β�����ʱ���
            }
            catch (sql::SQLException& e) {
                // �������ʧЧ�����´�������
                std::cout << "Error keeping connection alive: " << e.what() << std::endl;
                sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
                auto* newcon = driver->connect(url_, user_, pass_);  // ����������
                newcon->setSchema(schema_);
                con->_con.reset(newcon);  // �滻�ɵ�����
                con->_last_oper_time = timestamp;  // �������һ�β�����ʱ���
            }
        }
    }

    // �����ӳ��л�ȡһ������
    std::unique_ptr<SqlConnection> getConnection() {
        std::unique_lock<std::mutex> lock(mutex_);  // ������ȷ���̰߳�ȫ

        // �ȴ�ֱ�����ӳ����п������ӻ����ӳعر�
        cond_.wait(lock, [this] {
            if (b_stop_) {
                return true;
            }
            return !pool_.empty();
            });

        // ������ӳ��ѹرգ����ؿ�ָ��
        if (b_stop_) {
            return nullptr;
        }

        // ȡ��һ�����Ӳ�����
        std::unique_ptr<SqlConnection> con(std::move(pool_.front()));
        pool_.pop();
        return con;
    }

    // �����ӷ��ص����ӳ���
    void returnConnection(std::unique_ptr<SqlConnection> con) {
        std::unique_lock<std::mutex> lock(mutex_);  // ������ȷ���̰߳�ȫ

        // ������ӳ��ѹرգ�ֱ�ӷ���
        if (b_stop_) {
            return;
        }

        // �����ӷŻ����ӳأ���֪ͨ�ȴ����߳�
        pool_.push(std::move(con));
        cond_.notify_one();
    }

    // �ر����ӳ�
    void Close() {
        b_stop_ = true;  // ���ùرձ�־
        cond_.notify_all();  // ֪ͨ���еȴ����߳�
    }

    // �����������ͷ����ӳ��е���������
    ~MySqlPool() {
        std::unique_lock<std::mutex> lock(mutex_);  // ������ȷ���̰߳�ȫ
        while (!pool_.empty()) {
            pool_.pop();  // ������ӳ�
        }
    }

private:
    std::string url_;  // ���ݿ����� URL
    std::string user_;  // ���ݿ��û���
    std::string pass_;  // ���ݿ�����
    std::string schema_;  // ���ݿ�����
    int poolSize_;  // ���ӳش�С
    std::queue<std::unique_ptr<SqlConnection>> pool_;  // ���ӳأ�ʹ�ö��д洢���ӣ�
    std::mutex mutex_;  // �������������̰߳�ȫ
    std::condition_variable cond_;  // ���������������߳�ͬ��
    std::atomic<bool> b_stop_;  // ��־λ����ʾ���ӳ��Ƿ��ѹر�
    std::thread _check_thread;  // ��̨�̣߳����ڶ��ڼ������
};

//// UserInfo �ṹ�壺�洢�û���Ϣ
//struct UserInfo {
//    std::string name;  // �û���
//    std::string pwd;   // ����
//    int uid;           // �û� ID
//    std::string email; // ����
//};

// MysqlDao �ࣺ���ݿ���ʶ��󣬷�װ�˶� MySQL ���ݿ�Ĳ���
class MysqlDao {
public:
    MysqlDao();  // ���캯��
    ~MysqlDao(); // ��������

    // ע���û�
    int RegUser(const std::string& name, const std::string& email, const std::string& pwd);

    //// ʹ������ע���û�
    //int RegUserTransaction(const std::string& name, const std::string& email, const std::string& pwd, const std::string& icon);

    //// ��������Ƿ���ע��
    bool CheckEmail(const std::string& name, const std::string& email);

    //// �����û�����
    bool UpdatePwd(const std::string& name, const std::string& newpwd);

    //// ����û������Ƿ���ȷ
    bool CheckPwd(const std::string& name, const std::string& pwd, UserInfo& userInfo);

    //// ���Դ洢����
    //bool TestProcedure(const std::string& email, int& uid, std::string& name);

    std::shared_ptr<UserInfo> GetUser(int uid);

private:
    std::unique_ptr<MySqlPool> pool_;  // MySQL ���ӳ�
};
