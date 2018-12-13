#ifndef VERTEILER_THREADFIFO_HPP
#define VERTEILER_THREADFIFO_HPP

#include <queue>
#include <mutex>
#include <memory>

template <class T>
class ThreadFIFO
{

	std::queue<std::shared_ptr<T>> q;
	mutable std::mutex m;

public:

	bool get(std::shared_ptr<T>* zgr){
		std::unique_lock<std::mutex> lock(m);
		if(q.empty())
			return false;

		*zgr = q.front();
		q.pop();

		return true;
	}

	void put(T t){
		std::lock_guard<std::mutex> lock(m);
		q.push(std::make_shared<T>(t));
	}

};


#endif //VERTEILER_THREADFIFO_HPP
