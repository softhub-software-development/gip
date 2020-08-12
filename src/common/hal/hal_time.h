
//
//  hal_time.h
//
//  Created by Christian Lehner on 17/04/16.
//  Copyright (c) 2019 Softhub. All rights reserved.
//

#ifndef SOFTHUB_HAL_TIME_H
#define SOFTHUB_HAL_TIME_H

#include "hal.h"
#ifdef PLATFORM_WIN
#include <windows.h>
#include <winsock2.h>
#else
#include <sys/time.h>
#endif
#ifdef __OBJC__
#import <Foundation/Foundation.h>
#endif

namespace SOFTHUB {
namespace HAL {

#ifndef TIME_BOMB_ACTIVE
#define TIME_BOMB_ACTIVE 0
#endif

void time_bomb();

//
// class Timing
//

#ifdef PLATFORM_WIN
typedef LARGE_INTEGER Time_val;
#else
typedef struct timeval Time_val;
#endif

class Timing {

    mutable Time_val start;
    mutable Time_val stop;
    mutable long msec;

public:
    Timing();

    void begin() const;
    long end() const;

    class Block {

        std::string label;
        const Timing& timing;

        void print(long t);

    public:
        Block(const std::string& label, const Timing& timing);
        ~Block();
    };
};

}}

#endif
