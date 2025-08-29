#pragma once


template <typename T>
__forceinline T* zaddressof(T& t) {
    return reinterpret_cast<T*>(&const_cast<char&>(reinterpret_cast<const volatile char&>(t)));
}