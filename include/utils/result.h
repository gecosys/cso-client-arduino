#ifndef _UTILS_RESULT_H_
#define _UTILS_RESULT_H_

#include <utility>
#include <type_traits>
#include "error/error_code.h"

template <class T>
class Result {
public:
  Error::Code errorCode;
  T data;

public:
  Result() noexcept
    : errorCode(Error::Nil),
      data() {}

  Result(Error::Code& errorCode, T&& data) 
    noexcept(std::is_nothrow_move_constructible<T>::value)
      : errorCode(errorCode),
        data(std::forward<T>(data)) {}

  Result(Error::Code&& errorCode, T& data) 
    noexcept(std::is_nothrow_copy_constructible<T>::value)
      : errorCode(std::forward<Error::Code>(errorCode)),
        data(data) {}

  Result(Error::Code& errorCode, T& data) 
    noexcept(std::is_nothrow_copy_constructible<T>::value)
      : errorCode(errorCode),
        data(data) {}

  Result(Error::Code&& errorCode, T&& data) 
    noexcept(std::is_nothrow_move_constructible<T>::value)
      : errorCode(std::forward<Error::Code>(errorCode)),
        data(std::forward<T>(data)) {}

  Result(Result<T>&& other) 
    noexcept(std::is_nothrow_move_constructible<T>::value)
      : errorCode(std::forward<Error::Code>(other.errorCode)),
        data(std::forward<T>(other.data)) {}

  Result(const Result<T>& other) 
    noexcept(std::is_nothrow_copy_constructible<T>::value)
      : errorCode(other.errorCode),
        data(other.data) {}

  Result<T>& operator=(const Result<T>& other)
    noexcept(std::is_nothrow_copy_assignable<T>::value) {
      this->errorCode = other.errorCode;
      this->data = other.data;
      return *this;
  }

  Result<T>& operator=(Result<T>&& other)
    noexcept(std::is_nothrow_move_assignable<T>::value) {
      this->errorCode = std::forward<Error::Code>(other.errorCode);
      this->data = std::forward<T>(other.data);
      return *this;
  }
};

template <class T1, class T2>
constexpr Result<T2> make_result(T1&& t1, T2&& t2) {
  return Result<T2>(std::forward<T1>(t1), std::forward<T2>(t2));
}

#endif // _UTILS_RESULT_H_
