//
// Created by root on 28.11.18.
//

#ifndef STRING_ADD_HPP
#define STRING_ADD_HPP

#include <string>
#include <vector>

template <typename T>
inline static bool enthalten(T c, std::vector<T> satz){
	for(auto& i : satz)
		if(i == c)
			return true;
	return false;
}

inline static std::vector<std::string> string_split(std::string str, std::vector<char> trenner){

	if(str.empty())
		return {};

	int i;
	bool flag;

	flag = false;
	std::string tmp;
	std::vector<std::string> ret;

	for(i = 0; i < str.length(); ++i){
		if(!enthalten<char>(str.at(i), trenner)){
			flag = true;
			tmp += str.at(i);
		}else if(flag){
			ret.push_back(tmp);
			tmp.clear();
			flag = false;
		}
	}

	if(!tmp.empty())
		ret.push_back(tmp);

	return ret;

}

#endif