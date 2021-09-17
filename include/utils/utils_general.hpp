#ifndef UTILS_GENERAL_H
#define UTILS_GENERAL_H

#include <stdio.h>
#include <memory>
#include <string>
#include <stdexcept>

template<typename ... Args>
static std::string format(const std::string& format, Args&& ... args) {
    size_t size = snprintf(nullptr, 0, format.c_str(), args ...) + 1ULL; // Extra space for '\0'
    if (size <= 0) {
        throw "Error during formatting.";
    }
    std::unique_ptr<char> buf{ new char[size] };
    snprintf(buf.get(), size, format.c_str(), args ...);
    return std::string{ buf.get(), buf.get() + size - 1 }; // We don't want the '\0' inside
}

static std::string getMethodName(const std::string& name) noexcept {
	size_t begin = name.find_last_of("::");
    for (; begin > 0 && name[begin] != ' '; --begin);
    begin++;
    return name.substr(begin, name.rfind("(") - begin);
}

#define GET_FUNC_NAME() getMethodName(__PRETTY_FUNCTION__)

#endif // !UTILS_GENERAL_H