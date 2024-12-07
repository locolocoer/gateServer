#include "StatusConPool.h"


std::unique_ptr<StatusService::Stub> StatusConPool::getConnection()
{
	std::unique_lock<std::mutex> lock(_mutex);
	cv.wait(lock, [this]() {
		if (_b_stop) {
			return true;
		}
		return !_pool.empty();
		});
	if (_b_stop) {
		return nullptr;
	}
	auto ptr = std::move(_pool.front());
	_pool.pop();
	return std::move(ptr);
}

void StatusConPool::returnConection(std::unique_ptr<StatusService::Stub> con)
{
	std::lock_guard<std::mutex> lock(_mutex);
	if (_b_stop) {
		return;
	}
	_pool.push(std::move(con));
	cv.notify_one();
}

void StatusConPool::close()
{
	_b_stop = true;
	cv.notify_all();
}

StatusConPool::~StatusConPool()
{
	std::lock_guard<std::mutex> lock(_mutex);
	close();
	while (!_pool.empty()) {
		_pool.pop();
	}
}

StatusConPool::StatusConPool(std::string host, std::string port, int poolsize):_b_stop(false)
{
	for (int i = 0; i < poolsize; i++) {
		std::shared_ptr<grpc::Channel> _channel = grpc::CreateChannel(host + ":" + port, grpc::InsecureChannelCredentials());
		_pool.push(StatusService::NewStub(_channel));
	}
}

