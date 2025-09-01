#pragma once
#include "zalloc.h"


template <typename T>
class ZList : private ZRefCountedAccessor<T>, private ZRefCountedAccessor<ZRefCountedDummy<T>> {
private:
    size_t _m_uCount;
    T* _m_pHead;
    T* _m_pTail;

public:
    virtual ~ZList() {
        RemoveAll();
    }
    ZList() : _m_uCount(0), _m_pHead(nullptr), _m_pTail(nullptr) {
    }
    size_t GetCount() const {
        return _m_uCount;
    }
    int IsEmpty() const {
        return _m_uCount == 0;
    }
    T* GetHeadPosition() const {
        return _m_pHead;
    }
    T* GetTailPosition() const {
        return _m_pTail;
    }
    void RemoveAll() {
        auto p = _m_pHead;
        while (p) {
            auto pNext = GetNext(p);
            _Delete(pNext);
        }
        _m_pTail = nullptr;
        _m_pHead = nullptr;
        _m_uCount = 0;
    }
    T* FindIndex(size_t uIndex) {
        auto p = _m_pHead;
        for (size_t i = 0; i < uIndex; ++i)  {
            p = _GetNext(p);
        }
        return p;
    }

    static T& GetNext(T*& pos) {
        T& result = *pos;
        pos = pos ? _GetNext(pos) : nullptr;
        return result;
    }
    static T& GetPrev(T*& pos) {
        T& result = *pos;
        pos = pos ? _GetPrev(pos) : nullptr;
        return result;
    }

private:
    void _Delete(void* p) {
        auto pDummy = ZRefCountedDummy<T>::From(reinterpret_cast<T*>(p));
        ZRefCountedAccessorBase::_Delete(pDummy);
    }
    void _Delete(ZRefCounted* p) {
        ZRefCountedAccessorBase::_Delete(p);
    }

    static T* _GetNext(void* p) {
        auto pDummy = ZRefCountedDummy<T>::From(reinterpret_cast<T*>(p));
        auto pNext = ZRefCountedAccessor::_GetNext(pDummy);
        return pNext ? zaddressof(static_cast<ZRefCountedDummy<T>*>(pNext)->t) : nullptr;
    }
    static T* _GetNext(ZRefCounted* p) {
        return ZRefCountedAccessor::_GetNext(p);
    }
    static T* _GetPrev(void* p) {
        auto pDummy = ZRefCountedDummy<T>::From(reinterpret_cast<T*>(p));
        auto pPrev = ZRefCountedAccessor::_GetPrev(pDummy);
        return pPrev ? zaddressof(static_cast<ZRefCountedDummy<T>*>(pPrev)->t) : nullptr;
    }
    static T* _GetPrev(ZRefCounted* p) {
        return ZRefCountedAccessor::_GetPrev(p);
    }

};