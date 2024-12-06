#pragma once
#include "const.h"
#include "Singleton.h"
using IOContext = boost::asio::io_context;
using Work = IOContext::work;
using WorkPtr = std::unique_ptr<Work>;
class IOContextPool:public Singleton<IOContextPool>
{
	friend class Singleton<IOContextPool>;
public:
	IOContextPool(const IOContextPool&) = delete;
	IOContextPool& operator=(const IOContextPool&) = delete;
	~IOContextPool();
	void stop();
	IOContext& getIOContext();
private:
	IOContextPool(std::size_t size = std::thread::hardware_concurrency());
	std::vector<IOContext> _iocontexts;
	std::vector<WorkPtr> _workptrs;
	std::vector<std::thread> _threads;
	std::size_t next;
};

