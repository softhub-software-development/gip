
//
//  hal_lock.cpp
//
//  Created by Christian Lehner on 5/25/16.
//  Copyright (c) 2019 Softhub. All rights reserved.
//

#include "stdafx.h"
#include "hal_lock.h"
#include <errno.h>

namespace SOFTHUB {
namespace HAL {

//
// class Critical_section
//

Critical_section::Critical_section()
{
#ifdef PLATFORM_WIN
    InitializeCriticalSection(&mutex);
#else
    int error = pthread_mutex_init(&posix_mutex, 0);
    assert(!error);
#endif
}

Critical_section::~Critical_section()
{
#ifdef PLATFORM_WIN
    DeleteCriticalSection(&mutex);
#else
    int error = pthread_mutex_unlock(&posix_mutex);
    assert(!error);
    error = pthread_mutex_destroy(&posix_mutex);
    assert(!error || error == 16);  // TODO: seen it fail on raspberry for no apparent reason
#endif
}

void Critical_section::lock() const
{
#ifdef PLATFORM_WIN
    EnterCriticalSection(&mutex);
#else
    int error = pthread_mutex_lock(&posix_mutex);
    assert(!error);
#endif
}

void Critical_section::unlock() const
{
#ifdef PLATFORM_WIN
    LeaveCriticalSection(&mutex);
#else
    int error = pthread_mutex_unlock(&posix_mutex);
    assert(!error || error == 22);  // TODO: why does this happen at program termination?
#endif
}

//
// class Mutex
//

Mutex::Mutex()
{
#ifdef PLATFORM_WIN
    handle = CreateMutex(0, FALSE, 0);
#else
    pthread_mutexattr_t attr;
    int error = pthread_mutexattr_init(&attr);
    assert(!error);
    error = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    assert(!error);
    error = pthread_mutex_init(&posix_mutex, &attr);
    assert(!error);
    error = pthread_mutexattr_destroy(&attr);
    assert(!error);
#endif
}

Mutex::~Mutex()
{
#ifdef PLATFORM_WIN
    CloseHandle(handle);
#else
    int error = pthread_mutex_destroy(&posix_mutex);
    assert(!error || error == 22 || error == 16);  // may fail on shutdown
#endif
}

void Mutex::lock() const
{
#ifdef PLATFORM_WIN
    WaitForSingleObject(handle, INFINITE);
#else
    int error = pthread_mutex_lock(&posix_mutex);
    assert(!error || error == 22);  // may fail on shutdown
#endif
}

void Mutex::unlock() const
{
#ifdef PLATFORM_WIN
    ReleaseMutex(handle);
#else
    int error = pthread_mutex_unlock(&posix_mutex);
    assert(!error || error == 22);  // may fail on shutdown
#endif
}

}}
