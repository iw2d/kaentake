#pragma once
#include "ztl/ztl.h"
#include <type_traits>


template <typename T>
struct TSecData {
    T data;
    unsigned char bKey;
    unsigned char FakePtr1;
    unsigned char FakePtr2;
    unsigned short wChecksum;
};

static_assert(sizeof(TSecData<unsigned char>) == 0x6);
static_assert(sizeof(TSecData<unsigned int>) == 0xC);


template <typename T>
class TSecType {
protected:
    unsigned int FakePtr1;
    unsigned int FakePtr2;
    TSecData<T>* m_secdata;

public:
    explicit TSecType(const T op) : m_secdata(new TSecData<T>()) {
        FakePtr1 = rand();
        FakePtr2 = rand();
        m_secdata->FakePtr1 = FakePtr1;
        m_secdata->FakePtr2 = FakePtr2;
        SetData(op);
    }
    TSecType() : TSecType(0) {
    }
    ~TSecType() {
        delete m_secdata;
    }

    operator T() const {
        return GetData();
    }
    TSecType& operator=(const T& value) {
        SetData(value);
        return *this;
    }

    T GetData() const {
        T tmp;
        unsigned char bKey = m_secdata->bKey;
        unsigned short wChecksum = 0x9A65;
        for (size_t i = 0; i < sizeof(T); ++i) {
            if (!bKey) {
                bKey = 42;
            }
            unsigned char bEnc = reinterpret_cast<unsigned char*>(&m_secdata->data)[i];
            reinterpret_cast<unsigned char*>(&tmp)[i] = bEnc ^ bKey;
            bKey = bEnc + 42 + bKey;
            wChecksum = (wChecksum << 3) | (bKey + (wChecksum >> 13));
        }
        if (wChecksum != m_secdata->wChecksum || (FakePtr1 & 0xFF) != m_secdata->FakePtr1 || (FakePtr1 & 0xFF) != m_secdata->FakePtr1) {
            throw ZException(5);
        }
        return tmp;
    }
    void SetData(const T data) {
        m_secdata->bKey = rand();
        m_secdata->wChecksum = 0x9A65;
        unsigned char bKey = m_secdata->bKey;
        for (size_t i = 0; i < sizeof(T); ++i) {
            if (!bKey) {
                bKey = 42;
            }
            unsigned char bVal = reinterpret_cast<const unsigned char*>(&data)[i];
            reinterpret_cast<unsigned char*>(&m_secdata->data)[i] = bKey ^ bVal;
            bKey = (bKey ^ bVal) + 42 + bKey;
            m_secdata->wChecksum = (m_secdata->wChecksum << 3) | (bKey + (m_secdata->wChecksum >> 13));
        }
    }
};

static_assert(sizeof(TSecType<unsigned char>) == 0xC);
static_assert(sizeof(TSecType<unsigned int>) == 0xC);


template <typename T>
unsigned int __fastcall ZtlSecureTear(T* at, T t) {
    typedef std::conditional<(sizeof(T) < 4), unsigned char, unsigned int>::type V;
    constexpr unsigned int iteration = sizeof(T) / sizeof(V);
    constexpr unsigned int rotation = sizeof(T) < 4 ? 0 : 5;
    unsigned int checksum = 0xBAADF00D;
    V* v1 = reinterpret_cast<V*>(&at[0]);
    V* v2 = reinterpret_cast<V*>(&at[1]);
    V* value = reinterpret_cast<V*>(&t);
    for (size_t i = 0; i < iteration; ++i) {
        v1[i] = rand();
        v2[i] = _rotr(value[i] ^ v1[i], rotation);
        checksum = v2[i] + _rotr(v1[i] ^ checksum, 5);
    }
    return checksum;
}

template <typename T>
T __fastcall ZtlSecureFuse(T* at, unsigned int cs) {
    typedef std::conditional<(sizeof(T) < 4), unsigned char, unsigned int>::type V;
    constexpr unsigned int iteration = sizeof(T) / sizeof(V);
    constexpr unsigned int rotation = sizeof(T) < 4 ? 0 : 5;
    unsigned int checksum = 0xBAADF00D;
    V* v1 = reinterpret_cast<V*>(&at[0]);
    V* v2 = reinterpret_cast<V*>(&at[1]);
    V value[iteration] = { 0 };
    for (size_t i = 0; i < iteration; ++i) {
        value[i] = v1[i] ^ _rotl(v2[i], rotation);
        checksum = v2[i] + _rotr(v1[i] ^ checksum, 5);
    }
#ifdef _DEBUG
    assert(checksum == cs);
#endif
    return *reinterpret_cast<T*>(&value[0]);
}

template <typename T>
struct ZtlSecure {
    T at[2];
    unsigned int cs;

    operator T() {
        return ZtlSecureFuse<T>(at, cs);
    }
    ZtlSecure& operator=(const T& value) {
        cs = ZtlSecureTear<T>(at, t);
        return *this;
    }
};

#pragma pack(push, 1)
template <typename T>
struct ZtlSecurePacked {
    T at[2];
    unsigned int cs;

    operator T() {
        return ZtlSecureFuse<T>(at, cs);
    }
    ZtlSecurePacked& operator=(const T& value) {
        cs = ZtlSecureTear<T>(at, t);
        return *this;
    }
};
#pragma pack(pop)