#include "MysqlMgr.h"

int MysqlMgr::regUser(const std::string& usr, const std::string& email, const std::string& pwd)
{
    return _dao.regUser(usr,email,pwd);
}

MysqlMgr::~MysqlMgr()
{
}

MysqlMgr::MysqlMgr()
{
}
