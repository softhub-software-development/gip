
//
//  hal_lock.h
//
//  Created by Christian Lehner on 8/15/12.
//  Copyright (c) 2019 Softhub. All rights reserved.
//

#ifndef HAL_LOCK_H
#define HAL_LOCK_H

#include <base/base_platform.h>
#ifdef PLATFORM_WIN
#include <windows.h>
#else
#include <pthread.h>
#endif

#define LOCKED(mutex, stmt) { Lock::Block lock(mutex); stmt; }

namespace SOFTHUB {
namespace HAL {

//
// class Lock
//

class Lock {

public:
    virtual ~Lock() {}

    virtual void lock() const = 0;
    virtual void unlock() const = 0;

    class Block {

        const Lock& owner;

    public:
        Block(const Lock& lock) : owner(lock) { owner.lock(); }
        ~Block() { owner.unlock(); }
    };
};

//
// class Critical_section
//

class Critical_section : public Lock {

#ifdef PLATFORM_WIN
    mutable CRITICAL_SECTION mutex;
#else
    mutable pthread_mutex_t posix_mutex;
#endif

public:
    Critical_section();
    ~Critical_section();

    void lock() const;
    void unlock() const;
};

//
// class Mutex
//

class Mutex : public Lock {

    friend class Condition;
    
#ifdef PLATFORM_WIN
    mutable HANDLE handle;
#else
    mutable pthread_mutex_t posix_mutex;
#endif

public:
    Mutex();
    ~Mutex();

    void lock() const;
    void unlock() const;
};

}}

#endif
