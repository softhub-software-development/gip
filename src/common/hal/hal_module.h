
//
//  hal_module.h
//
//  Created by Christian Lehner on 8/15/12.
//  Copyright (c) 2019 Softhub. All rights reserved.
//

#ifndef SOFTHUB_HAL_MODULE_H
#define SOFTHUB_HAL_MODULE_H

#include "hal.h"

#if FEATURE_HAL_THREAD_POOL
#include "hal_thread_pool.h"
#ifndef MAX_POOL_THREADS
#ifdef _DEBUG
#define MAX_POOL_THREADS    12
#else
#define MAX_POOL_THREADS    32
#endif
#endif
#endif

namespace SOFTHUB {
namespace HAL {

//
// class Hal_module
//

class Hal_module : public BASE::Object<> {

#if FEATURE_HAL_THREAD_POOL
    static const int max_number_of_pool_threads = MAX_POOL_THREADS;

    Thread_pool* thread_pool;
#endif
    std::string module_file_name;

public:
    Hal_module();
    ~Hal_module();

    void configure(BASE::IConfig* config);
    const std::string& get_module_file_name() const { return module_file_name; }

#if FEATURE_HAL_THREAD_POOL
    Thread_pool* get_thread_pool() { return thread_pool; }
    void run(Runnable* runnable) { thread_pool->run(runnable); }
#endif

    static BASE::Module<Hal_module> module;
#ifdef _DEBUG
    static void test();
#endif
};

}}

#endif
