//
// Created by root on 29.11.18.
//

#ifndef VERTEILER_THREADFIFO_HPP
#define VERTEILER_THREADFIFO_HPP

#include <queue>
#include <mutex>
#include <memory>

// A threadsafe-queue.
template <class T>
class ThreadFIFO
{
public:
	ThreadFIFO(void)
			: q()
			, m()
	{}

	~ThreadFIFO(void)
	{}

	bool hole(std::shared_ptr<T>* zgr){
		std::unique_lock<std::mutex> lock(m);
		if(q.empty())
			return false;

		*zgr = q.front();
		q.pop();

		return true;
	}

	void setze(T t){
		std::lock_guard<std::mutex> lock(m);
		q.push(std::make_shared<T>(t));
	}

private:
	std::queue<std::shared_ptr<T>> q;
	mutable std::mutex m;
};


#endif //VERTEILER_THREADFIFO_HPP
