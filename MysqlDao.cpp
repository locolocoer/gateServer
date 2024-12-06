#include "MysqlDao.h"
#include "Defer.h"
#include "ConfigMgr.h"
MysqlConnection::MysqlConnection(sql::Connection* con_Ptr, long long op_time):_con(con_Ptr),_last_time_operation(op_time)
{
}

MysqlPool::MysqlPool(std::string url, std::string usr, std::string pwd, std::string schema, int poolsize):_b_stop(false),_url(url),_usr(usr),_pwd(pwd),_schema(schema),_poolsize(poolsize)
{
	try{
		for (int i = 0; i < poolsize; i++) {
			sql::mysql::MySQL_Driver* driver = sql::mysql::get_driver_instance();
			auto con = driver->connect(url, usr, pwd);
			con->setSchema(schema);
			auto currentTime = std::chrono::system_clock::now().time_since_epoch();
			long long timeStamp = std::chrono::duration_cast<std::chrono::seconds>(currentTime).count();
			_con_pool.push(std::make_unique<MysqlConnection>(con, timeStamp));
		}
		_thread = std::thread([this]() {
			while (!_b_stop) {
				checkConnection();
				std::this_thread::sleep_for(std::chrono::seconds(60));
			}
			});
		_thread.detach();
	}
	catch (sql::SQLException& e) {
		std::cout << "SQL connection failed!" << e.what() << std::endl;
	}
}

void MysqlPool::checkConnection()
{
	std::lock_guard<std::mutex> lock(_mutex);
	int poolsize = _con_pool.size();
	auto currentTime = std::chrono::system_clock::now().time_since_epoch();
	long long timeStamp = std::chrono::duration_cast<std::chrono::seconds>(currentTime).count();
	for (int i = 0; i < poolsize; i++) {
		auto con = std::move(_con_pool.front());
		_con_pool.pop();
		Defer defer([this, &con]() {
			_con_pool.push(std::move(con));
			});
		if (timeStamp - con->_last_time_operation < 5) {
			continue;
		}
		try {
			
			std::unique_ptr<sql::Statement> stmt(con->_con->createStatement());
			stmt->executeQuery("SELECT 1");
			con->_last_time_operation = timeStamp;
			std::cout << "keep the sql connect alive" << std::endl;
			
		}
		catch (sql::SQLException& e) {
			std::cout << "error keep alive: " << e.what() << std::endl;
			sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
			auto* newcon = driver->connect(_url, _usr, _pwd);
			newcon->setSchema(_schema);
			con->_con.reset(newcon);
			con->_last_time_operation = timeStamp;
		}
	}
}

std::unique_ptr<MysqlConnection> MysqlPool::getConnection()
{
	std::unique_lock<std::mutex> lock(_mutex);
	cv.wait(lock, [this]() {
		if (_b_stop) {
			return true;
		}
		return !_con_pool.empty();
		});
	if (_b_stop) {
		return nullptr;
	}
	std::unique_ptr<MysqlConnection> con(std::move(_con_pool.front()));
	_con_pool.pop();
	return std::move(con);
}

void MysqlPool::returnConnection(std::unique_ptr<MysqlConnection> con)
{
	std::unique_lock<std::mutex> lock(_mutex);
	if (_b_stop) {
		return;
	}
	_con_pool.push(std::move(con));
	cv.notify_one();
}

void MysqlPool::close()
{
	_b_stop = true;
	cv.notify_all();
}

MysqlPool::~MysqlPool()
{
	std::unique_lock<std::mutex> lock(_mutex);
	while (!_con_pool.empty()) {
		_con_pool.pop();
	}
}

MysqlDao::MysqlDao()
{
	auto cfg = ConfigMgr::Inst();
	std::string host =  cfg["Mysql"]["Host"];
	std::string usr = cfg["Mysql"]["User"];
	std::string port = cfg["Mysql"]["Port"];
	std::string pwd = cfg["Mysql"]["Passwd"];
	std::string schema = cfg["Mysql"]["Schema"];
	_pool.reset(new MysqlPool(host + ":" + port, usr, pwd, schema, 5));
}

MysqlDao::~MysqlDao()
{
	_pool->close();
}

int MysqlDao::regUser(std::string username, std::string email, std::string pwd)
{
	auto con = _pool->getConnection();
	try {
		if (con == nullptr) {
			return false;
		}
		// 准备调用存储过程
		std::unique_ptr<sql::PreparedStatement> stmt(con->_con->prepareStatement("CALL reg_user(?,?,?,@result)"));
		// 设置输入参数
		stmt->setString(1, username);
		stmt->setString(2, email);
		stmt->setString(3, pwd);
		// 由于PreparedStatement不直接支持注册输出参数，我们需要使用会话变量或其他方法来获取输出参数的值
		  // 执行存储过程
		stmt->execute();
		// 如果存储过程设置了会话变量或有其他方式获取输出参数的值，你可以在这里执行SELECT查询来获取它们
	   // 例如，如果存储过程设置了一个会话变量@result来存储输出结果，可以这样获取：
		std::unique_ptr<sql::Statement> stmtResult(con->_con->createStatement());
		std::unique_ptr<sql::ResultSet> res(stmtResult->executeQuery("SELECT @result AS result"));
		if (res->next()) {
			int result = res->getInt("result");
			std::cout << "Result: " << result << std::endl;
			_pool->returnConnection(std::move(con));
			return result;
		}
		_pool->returnConnection(std::move(con));
		return -1;
	}
	catch (sql::SQLException& e) {
		_pool->returnConnection(std::move(con));
		std::cerr << "SQLException: " << e.what();
		std::cerr << " (MySQL error code: " << e.getErrorCode();
		std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
		return -1;
	}
}
