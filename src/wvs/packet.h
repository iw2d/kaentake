#pragma once
#include "ztl/ztl.h"


class CInPacket {
protected:
    int m_bLoopback;
    int m_nState;
    ZArray<unsigned char> m_aRecvBuff;
    unsigned short m_uLength;
    unsigned short m_uRawSeq;
    unsigned short m_uDataLen;
    size_t m_uOffset;
};

static_assert(sizeof(CInPacket) == 0x18);


class COutPacket {
protected:
    int m_bLoopback;
    ZArray<unsigned char> m_aSendBuff;
    unsigned int m_uOffset;
    int m_bIsEncryptedByShanda;

public:
    explicit COutPacket(int nType) : m_aSendBuff(0x100) {
        Init(nType, 0, 0);
    }
    void Encode1(unsigned char n) {
        EncodeBuffer(&n, 1);
    }
    void Encode2(unsigned short n) {
        EncodeBuffer(&n, 2);
    }
    void Encode4(unsigned int n) {
        EncodeBuffer(&n, 4);
    }
    void EncodeStr(ZXString<char> s) {
        int n = s.GetLength();
        Encode2(n);
        EncodeBuffer(s, n);
    }
    void EncodeBuffer(const void* p, size_t uSize) {
        EnlargeBuffer(uSize);
        memcpy(&m_aSendBuff[m_uOffset], p, uSize);
        m_uOffset += uSize;
    }
    void Init(int nType, int bLoopback, int bTypeHeader1Byte) {
        m_bLoopback = bLoopback;
        m_uOffset = 0;
        if (nType != 0x7FFFFFFF) {
            if (bTypeHeader1Byte) {
                Encode1(nType);
            } else {
                Encode2(nType);
            }
        }
        m_bIsEncryptedByShanda = 0;
    }

protected:
    void EnlargeBuffer(size_t uSize) {
        size_t uCur = m_aSendBuff.GetCount();
        size_t uReq = m_uOffset + uSize;
        if (uCur < uReq) {
            do {
                uCur *= 2;
            } while (uCur < uReq);
            m_aSendBuff.Realloc(uCur, 0);
        }
    }
};

static_assert(sizeof(COutPacket) == 0x10);