#pragma once
#include "const.h"
#include <jdbc/mysql_driver.h>
#include <jdbc/mysql_connection.h>
#include <jdbc/cppconn/statement.h>
#include <jdbc/cppconn/prepared_statement.h>



class MysqlConnection {
public:
	std::unique_ptr<sql::Connection> _con;
	long long _last_time_operation;
	MysqlConnection(sql::Connection* con_Ptr,long long op_time);
};
class MysqlPool {
public:
	MysqlPool(std::string url,std::string usr,std::string pwd,std::string schema,int poolsize);
	void checkConnection();
	std::unique_ptr<MysqlConnection> getConnection();
	void returnConnection(std::unique_ptr<MysqlConnection> con);
	void close();
	~MysqlPool();
private:
	std::atomic<bool> _b_stop;
	std::mutex _mutex;
	std::condition_variable cv;
	std::queue<std::unique_ptr<MysqlConnection>> _con_pool;
	std::thread _thread;
	std::string _url, _usr, _pwd, _schema;
	int _poolsize;
};
class MysqlDao
{
public:
	MysqlDao();
	~MysqlDao();
	int regUser(std::string username, std::string email, std::string pwd);
	bool checkUsrMatchEmail(const std::string& usr, const std::string email);
	bool updatePwd(const std::string& email, const std::string& pwd);
	bool checkPwd(const std::string& name, const std::string& pwd, UserInfo& info);
	std::unique_ptr<MysqlPool> _pool;
};

