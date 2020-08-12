
//
//  net_observer.mm
//
//  Created by Christian Lehner on 2/23/18.
//  Copyright © 2019 softhub. All rights reserved.
//

// https://stackoverflow.com/questions/11240196/notification-when-wifi-connected-os-x

#import "net_observer.h"
#import <arpa/inet.h>
#import <ifaddrs.h>
#import <netdb.h>
#import <sys/socket.h>
#import <netinet/in.h>

#import <CoreFoundation/CoreFoundation.h>
#import <SystemConfiguration/SystemConfiguration.h>
#import <netinet/in.h>

@implementation NetObserver

static SCNetworkReachabilityRef networkReachability;
static BOOL isReachable;

// Observed flags:
// - nearly gone: kSCNetworkFlagsReachable alone (ignored)
// - gone: kSCNetworkFlagsTransientConnection | kSCNetworkFlagsReachable | kSCNetworkFlagsConnectionRequired
// - connected: kSCNetworkFlagsIsDirect | kSCNetworkFlagsReachable

static void networkReachabilityCallback(SCNetworkReachabilityRef target, SCNetworkConnectionFlags flags, void* object)
{
    if (!networkReachability)
        return;
    if ((flags & kSCNetworkFlagsReachable) && !(flags & kSCNetworkFlagsConnectionRequired)) {
        if (isReachable) // typically receive a reachable message ~20ms before the unreachable one
            return;
        isReachable = YES;
//      ncid_network_kill();
//      [NSThread detachNewThreadSelector : @selector(runThread:) toTarget : object withObject : nil];
    } else {
        isReachable = NO;
//      ncid_network_kill();
    }
}

- (void) checkReachability {
    NSString* server = [[NSUserDefaults standardUserDefaults] stringForKey : @"NCIDServer"];
    if (!server)
        return;
    const char* serverName = [[[server componentsSeparatedByString : @":"] objectAtIndex : 0] UTF8String];
    SCNetworkReachabilityContext context = {
         0, (__bridge void*) self, NULL, NULL, NULL
    };
    networkReachability = SCNetworkReachabilityCreateWithName(NULL, serverName);
    if (!networkReachability)
        return;
    SCNetworkConnectionFlags flags;
    if (SCNetworkReachabilityGetFlags(networkReachability, &flags))
        networkReachabilityCallback(networkReachability, flags, (__bridge void*) self);
    if (SCNetworkReachabilitySetCallback(networkReachability, networkReachabilityCallback, &context)) {
        if (SCNetworkReachabilityScheduleWithRunLoop(networkReachability, [[NSRunLoop currentRunLoop] getCFRunLoop], kCFRunLoopCommonModes)) {
            CFRelease(networkReachability);
            networkReachability = NULL;
        }
    }
}

@end

void testReachability()
{
    NetObserver* observer = [NetObserver new];
    [observer checkReachability];
}
