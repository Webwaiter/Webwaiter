// Copyright 2023 ean, hanbkim, jiyunpar

#include <vector>
#include <string>

std::vector<std::string> split(std::string input, std::string delimiter_set) {
    std::vector<std::string> result;
    std::string::size_type start = 0;
    std::string::size_type end = input.find_first_of(delimiter_set);
    while (end != std::string::npos) {
        if (end != start) {
            std::string token = input.substr(start, end - start);
            result.push_back(token);
        }
        start = end + 1;
        end = input.find_first_of(delimiter_set, start);
    }
    if (start != input.length()) {
        std::string token = input.substr(start);
        result.push_back(token);
    }
    return result;
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