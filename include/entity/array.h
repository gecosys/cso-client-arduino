#ifndef ENTITY_ARRAY_H
#define ENTITY_ARRAY_H

#include <tuple>
#include <cstring>

template<class T>
class Array {
private:
    T* buf;
    uint16_t size;

private:
    void free() noexcept {
      if (this->buf != nullptr) {
          delete[] this->buf;
      }
      this->buf = nullptr;
      this->size = 0;
    }

public:
    Array() noexcept
      : buf{ nullptr },
        size{ 0 } {}

    explicit Array(uint16_t size)
      : buf{ new T[size] },
        size{ size } {}

    // If isCopy = false, "Array" will manage memory as std::unique_ptr
    // If isCopy = true, "Array" will copy memory
    Array(T* buf, uint16_t size, bool isCopy = false) noexcept
      : buf{ nullptr },
        size{ size } {
        if (!isCopy) {
            this->buf = buf;
        }
        else {
            memcpy(this->buf, buf, size * sizeof(T));
        }
    }

    // Support for byte array init from string
    template <typename U,
              typename std::enable_if<std::is_same<U, char>::value &&
                                      std::is_same<T, uint8_t>::value>::type* = nullptr>
    explicit Array(const U* str)
      : buf{ nullptr },
        size{ strlen(str) + 1 } {
      this->buf = new T[this->size];
      memcpy(this->buf, str, this->size);
      this->buf[this->size - 1] = 0;
    }

    Array(const Array& other)
      : buf{ nullptr },
        size{ 0 } {
      if (other.size == 0 || other.buf == nullptr) {
          return;
      }
      this->size = other.size;
      this->buf = new T[other.size];
      memcpy(this->buf, other.buf, other.size);
    }

    Array(Array<T>&& other) noexcept
      : buf{ nullptr },
        size{ 0 } {
      std::swap(this->buf, other.buf);
      std::swap(this->size, other.size);
    }

    ~Array() noexcept {
      if (this->buf != nullptr) {
        delete[] this->buf;
      }
    }

    Array& operator=(const Array& other) {
      reset(other.size);
      if (other.size > 0) {
        memcpy(this->buf, other.buf, other.size);
      }
      return *this;
    }

    Array& operator=(Array&& other) noexcept {
      std::tie(this->buf, this->size) = other.release();
      return *this;
    }

    inline T& operator[](uint16_t index) const {
      return this->buf[index];
    }

    // Check array empty
    bool empty() const noexcept {
      return this->buf == nullptr;
    }

    // Return length array
    uint16_t length() const noexcept {
      return this->size;
    }

    // Return pointer array
    T* get() const noexcept {
      return this->buf;
    }

    // Reset array
    // If isCopy = false, "Array" will manage memory as std::unique_ptr
    // If isCopy = true, "Array" will copy memory
    void reset(size_t size = 0, T* buf = nullptr, bool isCopy = false) {
      free();
      if (size == 0) {
          return;
      }
      if (buf == nullptr) {
          buf = new T[size];
      }

      if (!isCopy) {
          this->buf = buf;
      }
      else {
          memcpy(this->buf, buf, size * sizeof(T));
      }
      this->size = size;
    }

    // Return pointer + length array
    std::tuple<T*, uint16_t> release() noexcept {
      T* buf = nullptr;
      uint16_t size = 0;
      std::swap(this->buf, buf);
      std::swap(this->size, size);
      return std::make_tuple(buf, size);
    }

    // Swap array
    void swap(Array<T>& other) noexcept {
      std::swap(this->buf, other.buf);
      std::swap(this->size, other.size);
    }
};

#endif // !ENTITY_ARRAY_H