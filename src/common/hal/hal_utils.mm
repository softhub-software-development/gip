
//
//  hal_utils.mm
//
//  Created by Christian Lehner on 20/04/16.
//  Copyright (c) 2019 Softhub. All rights reserved.
//

#import "hal_utils.h"

using namespace SOFTHUB::BASE;
using namespace std;

namespace SOFTHUB {
namespace HAL {

void os_version(string& platform, string& major, string& minor)
{
    NSString* osv = [[NSProcessInfo processInfo] operatingSystemVersionString];
#ifdef PLATFORM_MAC
    platform = "macOS";
#elif defined PLATFORM_IOS
    platform = "iOS";
#else
#error invalid platform
#endif
    const string& s = encode_utf8(osv);
    String_vector v;
    Strings::split(s, v);
    if (v.size() > 1) {
        const string& snum = v[1];
        v.clear();
        Strings::split(snum, v, ".");
        if (v.size() > 0)
            major = v[0];
        if (v.size() > 1)
            minor = v[1];
        if (v.size() > 2)
            minor += "." + v[2];
    } else {
        major = minor = "0";
    }
}

string encode_utf8(NSString* s)
{
    return s ? [s cStringUsingEncoding : NSUTF8StringEncoding] : "";
}

NSString* decode_utf8(const string& s)
{
    return [NSString stringWithCString : s.c_str() encoding : NSUTF8StringEncoding];
}

NSString* decode_mac_roman(const string& s)
{
    return [NSString stringWithCString : s.c_str() encoding : NSMacOSRomanStringEncoding];
}

NSDictionary* decode_json_dictionary(NSString* str, NSError* err)
{
    NSData* data = [str dataUsingEncoding : NSUTF8StringEncoding];
    return [NSJSONSerialization JSONObjectWithData : data options : NSJSONReadingMutableContainers error : &err];
}

NSString* encode_json_dictionary(NSDictionary* dict, NSError* err)
{
    NSData* data = [NSJSONSerialization dataWithJSONObject : dict options : NSJSONWritingPrettyPrinted error : &err];
    return err ? nil : [NSString stringWithUTF8String : (const char*) [data bytes]];
}

string full_user_name()
{
    return encode_utf8(NSFullUserName());
}

}}
