
//
//  net_observer.cpp
//
//  Created by Christian Lehner on 2/23/18.
//  Copyright © 2019 softhub. All rights reserved.
//

#include "stdafx.h"
#include "net_observer.h"

namespace SOFTHUB {
namespace NET {

//
// class Net_observer
//

Net_observer::Net_observer()
{
}

void Net_observer::test()
{
#ifdef PLATFORM_APPLE_TODO
    testReachability();
#endif
}

}}
