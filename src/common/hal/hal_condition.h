
//
//  hal.h
//
//  Created by Softhub.
//  Copyright (c) 2019 Softhub. All rights reserved.
//

#ifndef SOFTHUB_HAL_CONDITION_H
#define SOFTHUB_HAL_CONDITION_H

#include "hal_lock.h"

#ifndef INFINITE
#define INFINITE -1
#endif

namespace SOFTHUB {
namespace HAL {

class Mutex;

//
// class Condition
//

class Condition {

#ifdef PLATFORM_WIN
    HANDLE handle;
#else
    pthread_cond_t posix_condition;
#endif

public:
    Condition();
    ~Condition();
    
    bool wait(const Mutex& mutex, int time_in_ms = INFINITE);
    void signal();
};

}}

#endif
