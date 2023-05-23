// Copyright 2023 ean, hanbkim, jiyunpar

#include <vector>
#include <string>

std::vector<std::string> split(std::string input, std::string delimiter) {
	std::vector<std::string> ret;
	std::string token = "";
	size_t pos;
	while ((pos = input.find(delimiter)) != std::string::npos) {
		token = input.substr(0, pos);
		ret.push_back(token);
		input.erase(0, pos + delimiter.size());
	}
	ret.push_back(input);
	return ret;
}

std::string skipCharset(std::string input, std::string charset) {
	std::string ret;
	size_t pos;
	if ((pos = input.find_first_not_of(charset)) != std::string::npos) {
		ret = input.substr(pos);
		return ret;
	} else {
		return input;
	}
}