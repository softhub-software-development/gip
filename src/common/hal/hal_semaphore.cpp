
//
//  hal_semaphore.cpp
//
//  Created by Christian Lehner on 7/5/12.
//  Copyright (c) 2019 Softhub. All rights reserved.
//

#include "stdafx.h"
#include "hal_semaphore.h"
#include "hal_thread.h"
#include <errno.h>
#ifndef PLATFORM_WIN
#include <sys/time.h>
#endif

namespace SOFTHUB {
namespace HAL {

//
// class Semaphore
//

Semaphore::Semaphore(int count)
{
#ifdef PLATFORM_WIN
    handle = CreateSemaphore(0, count, count, 0);
#elif defined(PLATFORM_MAC) || defined(PLATFORM_IOS)
    handle = dispatch_semaphore_create(count);
    assert(handle);
#else
    int status = sem_init(&handle, 0, count);
    assert(status == 0);
#endif
}

Semaphore::~Semaphore()
{
#ifdef PLATFORM_WIN
    CloseHandle(handle);
#elif defined(PLATFORM_MAC) || defined(PLATFORM_IOS)
    dispatch_release(handle);
#else
    sem_destroy(&handle);
#endif
}

bool Semaphore::wait(int time_in_ms)
{
    bool timed_out = false;
#ifdef PLATFORM_WIN
    DWORD result = WaitForSingleObject(handle, time_in_ms);
    timed_out = result == WAIT_TIMEOUT;
#else
    if (time_in_ms == WAIT_INFINITE) {
#if defined(PLATFORM_MAC) || defined(PLATFORM_IOS)
        dispatch_semaphore_wait(handle, DISPATCH_TIME_FOREVER);
#else
        sem_wait(&handle);
#endif
    } else {
#if defined(PLATFORM_MAC) || defined(PLATFORM_IOS)
        timed_out = dispatch_semaphore_wait(handle, time_in_ms) != 0;
#else
        struct timeval tv;
        struct timespec ts;
        gettimeofday(&tv, NULL);
        ts.tv_sec = tv.tv_sec + time_in_ms / 1000;
        ts.tv_nsec = (tv.tv_usec + time_in_ms % 1000) / 1000;
        int status = sem_timedwait(&handle, &ts);
        timed_out = status == ETIMEDOUT;
#endif
    }
#endif
    return timed_out;
}

void Semaphore::signal()
{
#ifdef PLATFORM_WIN
    ReleaseSemaphore(handle, 1, 0);
#elif defined(PLATFORM_MAC) || defined(PLATFORM_IOS)
    dispatch_semaphore_signal(handle);
#else
    sem_post(&handle);
#endif
}

}}
