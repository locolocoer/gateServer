#include "IOContextPool.h"

IOContextPool::~IOContextPool()
{
	stop();
}

void IOContextPool::stop()
{
	for (std::size_t i = 0; i < _workptrs.size(); i++) {
		_workptrs[i]->get_io_context().stop();
		_workptrs[i].reset();
	}
	for (std::size_t i = 0; i < _threads.size(); i++) {
		_threads[i].join();
	}
}

IOContext& IOContextPool::getIOContext()
{
	std::size_t n = _iocontexts.size();
	next = (next + 1) % n;
	return _iocontexts[next];
}

IOContextPool::IOContextPool(std::size_t size):_iocontexts(size),_workptrs(size),next(0)
{
	for (std::size_t i = 0; i < size; i++) {
		_workptrs[i] = std::unique_ptr<Work>(new Work(_iocontexts[i]));
	}
	for (std::size_t i = 0; i < size; i++) {
		_threads.emplace_back([this, i]() {
			_iocontexts[i].run();
			});
	}
}
