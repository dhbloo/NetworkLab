#ifndef _HEADER_SYNCLOCK_
#define _HEADER_SYNCLOCK_
#pragma once

#include "util.h"

#include <mutex>
#include <ostream>

// IOÍ¬²½Ëø
class SyncLock
{
    std::mutex lock;

public:
    struct out_t
    {
        std::mutex &mtx;
    } out {lock};
    struct endl_t
    {
        std::mutex &mtx;
    } endl {lock};
    struct unlock_t
    {
        std::mutex &mtx;
    } unlock {lock};
};

inline std::ostream &operator<<(std::ostream &os, SyncLock::out_t lock)
{
    lock.mtx.lock();
    return os << TimeNow() << " | ";
}

inline std::ostream &operator<<(std::ostream &os, SyncLock::endl_t lock)
{
    os << std::endl;
    lock.mtx.unlock();
    return os;
}

inline std::ostream &operator<<(std::ostream &os, SyncLock::unlock_t lock)
{
    lock.mtx.unlock();
    return os;
}

#endif