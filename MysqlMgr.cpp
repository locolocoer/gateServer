#include "MysqlMgr.h"

int MysqlMgr::regUser(const std::string& usr, const std::string& email, const std::string& pwd)
{
    return _dao.regUser(usr,email,pwd);
}

bool MysqlMgr::checkUsrMatchEmail(const std::string& usr, const std::string& email)
{
    return _dao.checkUsrMatchEmail(usr,email);
}

bool MysqlMgr::updatePwd(const std::string& email, const std::string& pwd)
{
    return _dao.updatePwd(email,pwd);
}

MysqlMgr::~MysqlMgr()
{
}

MysqlMgr::MysqlMgr()
{
}
