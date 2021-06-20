
#include "stdafx.h"
#include "hal_thread.h"
#ifndef PLATFORM_WIN
#include <errno.h>
#include <unistd.h>
#include <sys/time.h>
#endif

using namespace SOFTHUB::BASE;

namespace SOFTHUB {
namespace HAL {

#ifndef PLATFORM_WIN

//
// class Thread_base, pthreads
//

void* Thread_base::invoke(void* arg)
{
    Thread_base* thread = static_cast<Thread_base*>(arg);
    assert(thread);
    thread->run_internal();
    return 0;
}

Thread_base::Thread_base() : stopped(true)
{
    int err = pthread_create(&posix_thread, 0, &Thread::invoke, this);
    assert(!err);
}

Thread_base::Thread_base(const pthread_t& pthread) : posix_thread(pthread), stopped(false)
{
}

Thread_base::~Thread_base()
{
    // TODO: can't detach if posix thread no more running
#if 0
    int error = pthread_detach(posix_thread);
    assert(!error || error == ESRCH);
#endif
}

void Thread_base::run_internal()
{
    if (stopped) {
        Lock::Block lock(mutex);
        condition.wait(mutex);
    }
    if (!stopped)
        run();
}

void Thread_base::start()
{
//  Lock::Block lock(mutex);
    stopped = false;
    condition.signal();
}

void Thread_base::stop()
{
//  Lock::Block lock(mutex);
    stopped = true;
    condition.signal();
}

void Thread_base::sleep(int time_in_ms)
{
    ::usleep(time_in_ms * 1000);
}

void Thread_base::join()
{
    int err = pthread_join(posix_thread, 0);
#ifdef _DEBUG
    char buf[1024];
    sprintf(buf, "join returned %d", err);
    log_message(ERR, buf);
#endif
    assert(!err || err == EDEADLK || err == EAGAIN);    // TODO
}

//
// class Thread, pthreads
//

Thread::Thread(const pthread_t& pthread) : Thread_base(pthread)
{
}

Thread Thread::current_thread()
{
    return Thread(pthread_self());
}

#endif

//
// class Thread, common
//

Thread::Thread(Runnable* target) : target(target)
{
}

Thread::~Thread()
{
}

void Thread::run()
{
    target->run();
    target = 0;
}

const Runnable* Thread::get_target()
{
    return target;
}

}}
