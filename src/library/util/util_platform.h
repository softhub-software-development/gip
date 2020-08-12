
//
//  util_platform.h
//
//  Created by Christian Lehner on 4/27/15.
//  Copyright (c) 2019 Softhub. All rights reserved.
//

#ifndef LIB_UTIL_PLATFORM_H
#define LIB_UTIL_PLATFORM_H

#include <base/base.h>

namespace SOFTHUB {
namespace UTIL {

class Platform {

public:
    static large total_memory_usage();
    static void print_total_memory_usage();
    static void print_process_memory_usage();
};

}}

#endif
