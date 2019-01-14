/*Copyright 2019, 2019 David A. Breunig

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

		http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

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
inline static bool vector_has(T c, const std::vector<T>& set) {
	for (auto& i : set)
		if (i == c)
			return true;
	return false;
}

inline static std::vector<std::string> string_split(const std::string& str, const std::vector<char>& splitter) {

	if (str.empty())
		return {};

	unsigned long i;
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
