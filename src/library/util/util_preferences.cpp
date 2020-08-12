
//
//  util_preferences.cpp
//
//  Created by Christian Lehner on 8/10/18.
//  Copyright (c) 2019 Softhub. All rights reserved.
//

#include "util_preferences.h"

using namespace std;

namespace SOFTHUB {
namespace UTIL {

Preferences_ref Preferences::instance(new Preferences());

#ifndef PLATFORM_APPLE

void Preferences::save(const string& key, const string& value)
{
    assert(!"TODO");
}

bool Preferences::restore(const string& key, string& value)
{
    assert(!"TODO");
}

#endif

}}
