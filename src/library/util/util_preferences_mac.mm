
//
//  util_preferences_mac.mm
//
//  Created by Christian Lehner on 8/10/18.
//  Copyright (c) 2019 Softhub. All rights reserved.
//

#include "util_preferences.h"
#include <hal/hal.h>
#import <Foundation/Foundation.h>

using namespace SOFTHUB::HAL;
using namespace std;

namespace SOFTHUB {
namespace UTIL {

//
// class Preferences
//

void Preferences::save(const string& key, const string& value)
{
    NSString* dictKey = decode_utf8(key);
    NSString* dictVal = decode_utf8(value);
    NSDictionary* defaults = [NSDictionary dictionaryWithObject : dictVal forKey : dictKey];
    [[NSUserDefaults standardUserDefaults] registerDefaults : defaults];
}

bool Preferences::restore(const string& key, string& value)
{
    NSString* dictKey = decode_utf8(key);
    NSString* dictVal = [[NSUserDefaults standardUserDefaults] objectForKey : dictKey];
    if (!dictVal)
        return false;
    value = encode_utf8(dictVal);
    return true;
}

}}
