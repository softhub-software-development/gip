
//
//  util_preferences.h
//
//  Created by Christian Lehner on 8/10/18.
//  Copyright (c) 2019 Softhub. All rights reserved.
//

#ifndef LIB_UTIL_PREFERENCES_H
#define LIB_UTIL_PREFERENCES_H

#include <base/base.h>

namespace SOFTHUB {
namespace UTIL {

FORWARD_CLASS(Preferences);

class Preferences : public BASE::Object<> {

private:
    Preferences() {}

public:
    void save(const std::string& key, const std::string& value);
    bool restore(const std::string& key, std::string& value);

    static Preferences_ref instance;
};

}}

#endif
