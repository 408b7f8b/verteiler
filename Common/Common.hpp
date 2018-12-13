#ifndef VERTEILER_COMMON_HPP
#define VERTEILER_COMMON_HPP

#include <queue>
#include <mutex>
#include <memory>
#include <iostream>
#include <string>
#include <vector>

template<class T>
class ThreadFIFO {

	std::queue<std::shared_ptr<T>> q;
	mutable std::mutex m;

public:

	bool get(std::shared_ptr<T>* zgr) {
		std::unique_lock<std::mutex> lock(m);
		if (q.empty())
			return false;

		*zgr = q.front();
		q.pop();

		return true;
	}

	void put(T t) {
		std::lock_guard<std::mutex> lock(m);
		q.push(std::make_shared<T>(t));
	}

};

//--------------------------------------------------------

template<typename T>
inline static bool vector_has(T c, std::vector<T> set) {
	for (auto& i : set)
		if (i == c)
			return true;
	return false;
}

inline static std::vector<std::string> string_split(std::string str, std::vector<char> splitter) {

	if (str.empty())
		return {};

	int i;
	bool flag;

	flag = false;
	std::string tmp;
	std::vector<std::string> ret;

	for (i = 0; i < str.length(); ++i) {
		if (!vector_has<char>(str.at(i), splitter)) {
			flag = true;
			tmp += str.at(i);
		} else if (flag) {
			ret.push_back(tmp);
			tmp.clear();
			flag = false;
		}
	}

	if (!tmp.empty())
		ret.push_back(tmp);

	return ret;

}

//--------------------------------------------------------

inline static void standard_logging(const std::string& strLogMsg) {
	std::cout << strLogMsg << std::endl;
}

#endif //VERTEILER_COMMON_HPP
