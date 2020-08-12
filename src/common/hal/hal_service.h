
//
//  hal_service.h
//
//  Created by Christian Lehner on 21/11/15.
//  Copyright (c) 2019 Softhub. All rights reserved.
//

#ifndef HAL_SERVICE_H
#define HAL_SERVICE_H

#include "hal_module.h"
#include "hal_thread.h"
#include <sstream>

namespace SOFTHUB {
namespace HAL {

FORWARD_CLASS(IService);

typedef enum {
    state_inactive = 0, state_initializing, state_active, state_stopped, state_terminated
} Server_state;

//
// class IService
//

class IService : public Runnable {

public:
    virtual void service_loop() = 0;

    static void run(IService* service);
};

//
// class Service
//

template <typename T = IService>
class Service : public BASE::Object<T> {

    Mutex mutex;

protected:
    Server_state state;

    virtual void service_control_event() {}

public:
    Service() : state(state_inactive) {}

    virtual void start();
    virtual void stop();
    virtual bool is_stopped() const { return state == state_stopped; }
    virtual bool is_active() const { return state == state_active; }
    virtual void service_loop();
    virtual void run();
    virtual void fail(const std::exception& ex) {}
    virtual void report_error(const std::string& msg) const;
    virtual void release_memory() {}

    virtual bool initialize() = 0;
    virtual void finalize() = 0;
    virtual void serve_request() = 0;
};

template <typename T>
void Service<T>::start()
{
    service_control_event();
    T::run(this);
}

template <typename T>
void Service<T>::stop()
{
    state = state_stopped;
    service_control_event();
}

template <typename T>
void Service<T>::run()
{
    service_control_event();
    this->service_loop();
    state = state_terminated;
    service_control_event();
}

template <typename T>
void Service<T>::service_loop()
{
    while (!is_stopped()) {
        Thread::sleep(5);
        if (state == state_inactive) {
            if (initialize())
                state = state_active;
        }
        if (state == state_active) {
            try {
                serve_request();
            } catch (BASE::Exception& ex) {
                std::stringstream stream;
                stream << "service: " << ex.get_message();
                report_error(stream.str());
                if (!is_stopped())
                    state = state_inactive;
                finalize();
            }
        }
    }
    state = state_inactive;
    finalize();
}

template <typename T>
void Service<T>::report_error(const std::string& msg) const
{
    std::cerr << msg << std::endl;
}

}}

#endif
