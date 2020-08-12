
//
//  hal_condition.cpp
//
//  Created by Softhub.
//  Copyright (c) 2019 Softhub. All rights reserved.
//

#include "stdafx.h"
#include "hal_condition.h"
#include "hal_lock.h"
#include "hal_utils.h"
#include <errno.h>
#include <string.h>

namespace SOFTHUB {
namespace HAL {

//
// class Condition
//

Condition::Condition()
{
#ifdef PLATFORM_WIN
    handle = CreateEvent(0, 0, 0, 0);
#else
    int error = pthread_cond_init(&posix_condition, 0);
    assert(!error);
#endif
}

Condition::~Condition()
{
#ifdef PLATFORM_WIN
    CloseHandle(handle);
#else
    int error = pthread_cond_destroy(&posix_condition);
    assert(!error);
#endif
}

bool Condition::wait(const Mutex& mutex, int time_in_ms)
{
    bool timed_out;
#ifdef PLATFORM_WIN
    DWORD result = WaitForSingleObject(handle, time_in_ms);
    timed_out = result == WAIT_TIMEOUT;
#else
    if (time_in_ms == INFINITE) {
        int err = pthread_cond_wait(&posix_condition, &mutex.posix_mutex);
        if (err)
            log_message(ERR, strerror(errno));
        timed_out = false;
    } else {
        struct timeval tv;
        struct timespec ts;
        gettimeofday(&tv, NULL);
        ts.tv_sec = tv.tv_sec + time_in_ms / 1000;
        ts.tv_nsec = (tv.tv_usec + time_in_ms % 1000) / 1000;
        int status = pthread_cond_timedwait(&posix_condition, &mutex.posix_mutex, &ts);
        assert(!status || status == ETIMEDOUT);
        timed_out = status == ETIMEDOUT;
    }
#endif
    return timed_out;
}

void Condition::signal()
{
#ifdef PLATFORM_WIN
    SetEvent(handle);
#else
    int error = pthread_cond_signal(&posix_condition);
    assert(!error || error == 22);
#endif
}

}}
