
//
//  net_observer.h
//
//  Created by Christian Lehner on 2/23/18.
//  Copyright © 2019 softhub. All rights reserved.
//

#ifndef NET_OBSERVER_H
#define NET_OBSERVER_H

#include <base/base.h>

#ifdef __OBJC__

#import <Foundation/Foundation.h>

@interface NetObserver : NSObject {
}

- (void) checkReachability;

@end

#endif

#ifdef PLATFORM_APPLE

extern void testReachability();

#endif

namespace SOFTHUB {
namespace NET {

FORWARD_CLASS(Net_observer);

//
// class Net_observer
//

class Net_observer : public BASE::Object<> {

public:
    Net_observer();

    void test();
};

}}

#endif

