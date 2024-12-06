#pragma once
#include "Singleton.h"
#include "MysqlDao.h"
class MysqlMgr:public Singleton<MysqlMgr>
{
	friend class Singleton<MysqlMgr>;
public:
	int regUser(const std::string& usr, const std::string& email, const std::string& pwd);
	~MysqlMgr();
private:
	MysqlDao _dao;
	MysqlMgr();
};

