#pragma once
#include <cstdint>


template <typename T, uintptr_t ADDRESS>
class TSingleton {
public:
    inline static T*& ms_pInstance = *reinterpret_cast<T**>(ADDRESS);

    static T* GetInstance() {
        return ms_pInstance;
    }
    static bool IsInstantiated() {
        return ms_pInstance != nullptr;
    }
};