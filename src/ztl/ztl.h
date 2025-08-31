#pragma once

template <typename T>
__forceinline T zmax(T a, T b) {
    return b < a ? a : b;
}

template <typename T>
__forceinline T zmin(T a, T b) {
    return a < b ? a : b;
}

template <typename T>
__forceinline T zclamp(T value, T min, T max) {
    return zmax(min, zmin(max, value));
}

template <typename T>
__forceinline T* zaddressof(T& t) {
    return reinterpret_cast<T*>(&const_cast<char&>(reinterpret_cast<const volatile char&>(t)));
}