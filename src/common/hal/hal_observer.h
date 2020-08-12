
#ifndef HAL_OBSERVER_H
#define HAL_OBSERVER_H

#include <base/base.h>
#ifdef PLATFORM_MAC
#include <CoreFoundation/CoreFoundation.h>
#endif

namespace SOFTHUB {
namespace HAL {

FORWARD_CLASS(INotification);
FORWARD_CLASS(IObserver);

//
// interface INotification
//

class INotification : public BASE::Interface {

public:
    virtual void on_observation(IObserver* observer) = 0;
};

//
// interface IObserver
//

class IObserver : public BASE::Interface {

public:
    virtual bool observe(bool synchronously = false) = 0;
};

//
// class Kill_switch_observer
//

class Kill_switch_observer : public BASE::Object<IObserver> {

    INotification_ref target;
#ifdef PLATFORM_MAC
    CFNotificationCenterRef notification_center;
    CFRunLoopRef run_loop;

    static void callback(CFNotificationCenterRef center, void* data, CFStringRef name, const void* obj, CFDictionaryRef userInfo);
#endif

protected:
    void notify();

public:
    Kill_switch_observer(INotification* target) : target(target) {}

    bool observe(bool synchronously = false);
};

}}

#endif

