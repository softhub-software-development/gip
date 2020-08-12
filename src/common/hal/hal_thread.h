
#ifndef HAL_THREAD_H
#define HAL_THREAD_H

#include <base/base.h>
#include "hal_condition.h"
#include "hal_lock.h"

namespace SOFTHUB {
namespace HAL {

FORWARD_CLASS(Runnable);
FORWARD_CLASS(Thread);

//
// class Thread_base
//

class Thread_base {

#ifdef PLATFORM_WIN
    static DWORD invoke(void*);
#else
    static void* invoke(void*);
#endif

    bool stopped;

    void run_internal();

    virtual void run() = 0;

#ifdef PLATFORM_WIN
    HANDLE handle;
    HANDLE evt;
    DWORD id;

protected:
    Thread_base(HANDLE handle, DWORD id);
#else
protected:
	pthread_t posix_thread;

	Thread_base(const pthread_t& pthread);
#endif
    Mutex mutex;
    Condition condition;

public:
    Thread_base();
    virtual ~Thread_base();

#ifdef PLATFORM_WIN
    bool operator==(const Thread_base& thread) { return id == thread.id; }
#endif

    virtual void start();
    virtual void stop();

    bool is_stopped() const { return stopped; }
    void join();

    static void sleep(int time_in_ms);
};

//
// interface Runnable
//

class Runnable : public virtual BASE::Interface {

public:
    virtual void run() = 0;
    virtual void fail(const std::exception& ex) = 0;
};

//
// class Thread
//

class Thread : public Thread_base {

    Runnable_ref target;

protected:
#ifdef PLATFORM_WIN
    Thread(Runnable* target, HANDLE handle, DWORD id);
#else
    Thread(const pthread_t& pthread);
#endif
    void run();

public:
    Thread(Runnable* target);
    ~Thread();

    const Runnable* get_target();
    static Thread current_thread();
};

}}

#endif

