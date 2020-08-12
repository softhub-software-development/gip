
//
//  net_discovery.cpp
//
//  Created by Christian Lehner on 20/04/16.
//  Copyright (c) 2019 Softhub. All rights reserved.
//

#include "stdafx.h"
#include "net.h"

namespace SOFTHUB {
namespace NET {

using namespace HAL;

Discovery::Discovery()
{
}

Discovery::~Discovery()
{
}

void Discovery::add_listener(Notification* listener)
{
    listeners.push_back(listener);
}

void Discovery::remove_listener(Notification* listener)
{
#ifdef PLATFORM_WIN
    // TODO: this is bullshit
    Notification_vector::const_iterator iterator = listeners.begin();
    Notification_vector::const_iterator end = listeners.end();
    while (iterator != end) {
        Notification* tmp = *iterator;
        if (tmp == listener) {
            listeners.erase(iterator);
            break;
        }
        iterator++;
    }
#endif
}

void Discovery::run()
{
    while (true)
        Thread::sleep(1);
}

}}

