
//
//  hal_semaphore.h
//
//  Created by Christian Lehner on 7/5/12.
//  Copyright (c) 2019 Softhub. All rights reserved.
//

#ifndef HAL_SEMAPHORE_H
#define HAL_SEMAPHORE_H

#include "hal_lock.h"
#include "hal_condition.h"
#ifdef PLATFORM_WIN
#include <windows.h>
#define WAIT_INFINITE INFINITE
#elif defined(PLATFORM_MAC) || defined(PLATFORM_IOS)
#include <dispatch/dispatch.h>
#define WAIT_INFINITE INFINITE
#else
#include <semaphore.h>
#define WAIT_INFINITE 0
#endif

namespace SOFTHUB {
namespace HAL {

//
// class Semaphore
//

class Semaphore {

#ifdef PLATFORM_WIN
    HANDLE handle;
#elif defined(PLATFORM_MAC) || defined(PLATFORM_IOS)
    dispatch_semaphore_t handle;
#else
    sem_t handle;
#endif

public:
    Semaphore(int count = 0);
    ~Semaphore();

    bool wait(int time_in_ms = WAIT_INFINITE);
    void signal();
};

}}

#endif
