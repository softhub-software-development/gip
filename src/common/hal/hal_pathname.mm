
//
//  hal_pathname.mm
//
//  Created by Christian Lehner on 12/26/16.
//  Copyright (c) 2019 Softhub. All rights reserved.
//

#include "hal_pathname.h"
#include "hal_utils.h"

using namespace std;

namespace SOFTHUB {
namespace HAL {

#ifdef PLATFORM_APPLE

Pathname* Pathname::from_url(NSURL* url)
{
    NSString* path = [url path];
    string str = encode_utf8(path);
    return Pathname::from_platform_string(str);
}

#endif

}}
