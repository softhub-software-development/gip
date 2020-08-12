
#include "stdafx.h"
#include "hal_thread.h"

#ifdef PLATFORM_WIN

using namespace SOFTHUB::BASE;

namespace SOFTHUB {
namespace HAL {

//
// class Thread_base, Windows
//

Thread_base::Thread_base() : evt(0), stopped(false)
{
    LPTHREAD_START_ROUTINE proc = (LPTHREAD_START_ROUTINE) &Thread::invoke;
    LPVOID parameter = this;
    handle = CreateThread(NULL, 0, proc, parameter, CREATE_SUSPENDED, &id);
}

Thread_base::Thread_base(HANDLE handle, DWORD id)
  : handle(handle), evt(0), id(id), stopped(false)
{
}

Thread_base::~Thread_base()
{
    CloseHandle(handle);
    if (evt)
        CloseHandle(evt);
}

void Thread_base::start()
{
    ResumeThread(handle);
}

void Thread_base::stop()
{
    SuspendThread(handle);
}

void Thread_base::sleep(int time_in_ms)
{
    Sleep(time_in_ms);
}

DWORD Thread_base::invoke(void* arg)
{
    Thread_base* thread = static_cast<Thread_base*>(arg);
    thread->run();
    return 0;
}

//
// class Thread, Windows
//

Thread::Thread(Runnable* target, HANDLE handle, DWORD id)
  : target(target), Thread_base(handle, id)
{
}

Thread Thread::current_thread()
{
    return Thread(0, GetCurrentThread(), GetCurrentThreadId());
}

void Thread_base::join()
{
    WaitForSingleObject(handle, INFINITE);
}

}}

#endif
