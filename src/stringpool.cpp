#include "pch.h"
#include "hook.h"
#include "ztl/ztl.h"

#define REPLACE_STRING(INDEX, NEW_STRING) \
    do { \
        static char sEncoded[GetLength(NEW_STRING) + 2]; \
        EncodeString(INDEX, NEW_STRING, sEncoded); \
    } while (0)


class StringPool {
public:
    inline static auto ms_aKey = reinterpret_cast<const unsigned char*>(0x00B001EC);
    inline static auto ms_aString = reinterpret_cast<const char**>(0x00BDC9D4);

    class Key {
    public:
        ZArray<unsigned char> m_aKey;
        Key(const unsigned char* pKey, unsigned int nKeySize, unsigned int nSeed) {
            reinterpret_cast<void(__thiscall*)(Key*, const unsigned char*, unsigned int, unsigned int)>(0x0079E780)(this, pKey, nKeySize, nSeed);
        }
    };
    static_assert(sizeof(Key) == 0x4);
};

constexpr size_t GetLength(const char* s) {
    size_t n = 0;
    while (s[n]) {
        ++n;
    }
    return n;
}

void EncodeString(int nIdx, const char* sSource, char* sDestination) {
    StringPool::Key keygen(StringPool::ms_aKey, 0x10, 0);
    size_t n = strlen(sSource);
    for (size_t i = 0; i < n; ++i) {
        unsigned char key = keygen.m_aKey[i % 0x10];
        sDestination[i + 1] = sSource[i] ^ key;
        if (sSource[i] == key) {
            sDestination[i + 1] = key;
        }
    }
    sDestination[0] = 0;
    sDestination[n + 1] = 0;
    StringPool::ms_aString[nIdx] = sDestination;
}


void AttachStringPoolMod() {
    REPLACE_STRING(1163, "Kaentake");
}