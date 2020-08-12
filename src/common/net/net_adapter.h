
//
//  net_adapter.h
//
//  Created by Christian Lehner on 20/04/16.
//  Copyright (c) 2019 Softhub. All rights reserved.
//

#ifndef NET_CLIENT_H
#define NET_CLIENT_H

#include "net_err.h"
#include <hal/hal_thread.h>
#include <string>
#include <vector>
#ifdef PLATFORM_WIN
#include <winsock2.h>
#else
#include <sys/socket.h>
#endif

struct sockaddr;

namespace SOFTHUB {
namespace NET {

class Address;
class Socket_mcast;
#ifdef _DEBUG
class Monkey;
#endif

class Net_adapter : public BASE::Object<> {

    Address* local_address;
    Address* group_address;
    Socket_mcast* socket;
    std::string hostname;
    Status status;
#ifdef _DEBUG
    HAL::Thread* monkey;
    void init_remote_mockup();
    void dispose_remote_mockup();
#endif

public:
    Net_adapter();
    ~Net_adapter();

    const Address* get_local_address() { return local_address; }
    const Address* get_group_address() { return group_address; }
    Status get_status() { return status; }

    static void test();
};

class Packet {

public:
    Packet();
};

class Event {

public:
    Event();
};

class Notification {

public:
    virtual void callback(const Event* evt) = 0;
};

class Discovery : public BASE::Object<HAL::Runnable> {

    typedef std::vector<Notification*> Notification_vector;

    Notification_vector listeners;

    void run();

public:
    Discovery();
    ~Discovery();

    void add_listener(Notification* listener);
    void remove_listener(Notification* listener);
};

#ifdef _DEBUG

class Monkey : public BASE::Object<HAL::Runnable> {

    Address* local_address;

    void run();

public:
    Monkey(Address* local_address);
    ~Monkey();
};

#endif

}}

#endif

