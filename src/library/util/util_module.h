
/*
 *  util.h
 *
 *  Created by Christian Lehner on 8/17/10.
 *  Copyright 2018 softhub. All rights reserved.
 */

#ifndef LIB_UTIL_MODULE_H
#define LIB_UTIL_MODULE_H

#include "util.h"
#include <base/base_module.h>

namespace SOFTHUB {
namespace UTIL {

class Util_module : public BASE::Object<> {

public:
    Util_module();
    ~Util_module();

    static BASE::Module<Util_module> module;
#ifdef _DEBUG
    static void test();
#endif
};

}}

#endif
