#ifndef _UTILS_SAFE_H_
#define _UTILS_SAFE_H_

#include <cstdint>

/*
*
* ESP32 module doesn't support "new (std::nothrow)"" to check successfull allocation memory
* Therefor, we have to try-cacth when call operator "new"
* See more at link:
    "https://arduino-esp8266.readthedocs.io/en/latest/reference.html" (Progmem section)
* "Safe" class will include safety functions include allocate memory with operator "new"
* You can use "std::malloc" to allocate for basic types but not be able for custom types
* because it doesn's call constructor
* "new_arr" and "new_obj" are more general
*
*/

class Safe {
public:
    // Dynamic allocate array
    template<typename T>
    static T* new_arr(std::size_t size) {
        try {
            return new T[size];
        } 
        catch (const std::bad_alloc& e) {
            return nullptr;
        }
    }

    // Dynamic allocate object 
    // Inclue default constructor, parameters constructor, move constructor, copy constructor
    template<typename T, class... Args>
    static T* new_obj(Args&&... args) {
        try {
            return new T(std::forward<Args>(args)...);
        } 
        catch (const std::bad_alloc& e) {
            return nullptr;
        }
    }
};

#endif //_UTILS_ALLOCATE_H_