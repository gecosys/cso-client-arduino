#ifndef ERROR_H
#define ERROR_H

#include <tuple>
#include <string>
#include <cstdint>

// The exception handling:
//      1. Just throw an exception in constructors and some operators if it is really necessary.
//         Return error codes in other methods.
//      2. Should throw a string instead of a std::exception 
//         because a std:exception can throw another unexpected error especially from its copy constructors.
//      3. The thrown string should include the error location (class, function) and 
//         the error reason for easy tracking
// The reason for doing this is that throwing exceptions is discouraged especially in embedded systems.

class Error {
private:
	struct Information {
		std::string funcName;
		std::string content;
	};

private:
	Information* info;

public:
	Error() noexcept;
	Error(std::string&& funcName, std::string&& content) noexcept;
	Error(Error&& other) noexcept;
	Error(const Error& other);
	~Error() noexcept;

	Error& operator=(Error&& other) noexcept;
	Error& operator=(const Error& other);

	bool nil() const noexcept;
	std::string toString() const noexcept;
};

#endif // !ERROR_H