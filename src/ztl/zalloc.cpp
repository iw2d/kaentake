#include "zalloc.h"
#include <windows.h>


void* ZAllocBase::_AllocRaw(size_t uSize) {
    HANDLE hHeap = GetProcessHeap();
    auto pAlloc = static_cast<size_t*>(HeapAlloc(hHeap, 0, uSize + sizeof(size_t)));
    *pAlloc = uSize;
    return pAlloc + 1;
}

void ZAllocBase::_FreeRaw(void* p) {
    HANDLE hHeap = GetProcessHeap();
    HeapFree(hHeap, 0, static_cast<size_t*>(p) - 1);
}

void* ZAllocBase::_AllocRawBlocks(size_t uBlockSize, size_t u) {
    size_t uAllocSize = uBlockSize + sizeof(size_t);
    auto pAlloc = static_cast<size_t*>(_AllocRaw(u * uAllocSize + sizeof(size_t)));
    *pAlloc++ = 0;
    *pAlloc = uBlockSize;
    size_t* pBlock = pAlloc + 1;
    for (size_t nRemain = u - 1; nRemain > 0; --nRemain) {
        *pBlock = reinterpret_cast<uintptr_t>(pBlock) + uAllocSize;
        pBlock = reinterpret_cast<size_t*>(*pBlock);
        *(pBlock - 1) = uBlockSize;
    }
    *pBlock = 0;
    return pAlloc + 1;
}

void ZAllocBase::_FreeRawBlocks(void* p) {
    if (!p) {
        return;
    }
    _FreeRaw(static_cast<size_t*>(p) - 2);
}

size_t ZAllocBase::_MemSize(void* p) {
    size_t uSize = *(static_cast<size_t*>(p) - 1);
    if (uSize & 0x80000000) {
        uSize = ~uSize;
    }
    return uSize;
}

void*& ZAllocBase::_NextHeadBlock(void* p) {
    return *reinterpret_cast<void**>(static_cast<size_t*>(p) - 2);
}


ZRefCounted* _Set1(ZRefCounted* p) {
    InterlockedExchange(&p->_m_nRef, 1);
    return p;
}

void ZRefCountedAccessorBase::_Delete(ZRefCounted* p) {
    delete p;
}

long ZRefCountedAccessorBase::_AddRef(ZRefCounted* p) {
    return InterlockedIncrement(&p->_m_nRef);
}

long ZRefCountedAccessorBase::_Release(ZRefCounted* p) {
    return InterlockedDecrement(&p->_m_nRef);
}

ZRefCounted*& ZRefCountedAccessorBase::_GetPrev(ZRefCounted* p) {
    return p->_m_pPrev;
}

ZRefCounted*& ZRefCountedAccessorBase::_GetNext(ZRefCounted* p) {
    return p->_m_pNext;
}