
#include "stdafx.h"
#include "hal_observer.h"

using namespace SOFTHUB::BASE;

namespace SOFTHUB {
namespace HAL {

//
// class Kill_switch_observer
//

bool Kill_switch_observer::observe(bool synchronously)
{
#ifdef PLATFORM_MAC
    CFNotificationCenterRef notification_center = CFNotificationCenterGetDistributedCenter();
    if (!notification_center)
        return false;
    CFStringRef evt_name = CFSTR("com.apple.logoutInitiated");
    CFNotificationCenterAddObserver(notification_center, this, &callback, evt_name, NULL, CFNotificationSuspensionBehaviorDeliverImmediately);
    if (synchronously) {
        CFRunLoopRun();
        run_loop = CFRunLoopGetCurrent();
    }
    return true;
#else
    return false;
#endif
}

void Kill_switch_observer::notify()
{
    target->on_observation(this);
#ifdef PLATFORM_MAC
    if (run_loop) {
        CFRunLoopStop(run_loop);
        run_loop = 0;
    }
#endif
}

#ifdef PLATFORM_MAC

void Kill_switch_observer::callback(CFNotificationCenterRef center, void* data, CFStringRef name, const void* obj, CFDictionaryRef userInfo)
{
    Kill_switch_observer* observer = (Kill_switch_observer*) data;
    assert(observer);
    observer->notify();
}

#endif

}}
