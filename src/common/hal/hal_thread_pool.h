
#ifndef HAL_THREAD_POOL_H
#define HAL_THREAD_POOL_H

#include "hal_thread.h"
#include "hal_condition.h"
#include "hal_semaphore.h"
#include "hal_lock.h"
#include <vector>

namespace SOFTHUB {
namespace HAL {

class Pool_thread;
class Runnable;

typedef BASE::Vector<Pool_thread*> Pool_thread_vector;

//
// class Thread_pool
//

class Thread_pool {

    friend class Pool_thread;

    static const int out_of_pool_threads_retries = 12;
    static const int reserve_memory_size = 1 << 20;

    Mutex mutex;
    Condition condition;
    Pool_thread_vector threads;
    byte* reserve_memory;
    int num_threads;
    int num_active_threads;
    int num_waiting_threads;
    int num_jobs;
    int num_errors;
    bool terminated;

    void terminate(Pool_thread* thread);
    void execute(Runnable* target);
    void last_resort();

    template<typename T> void inc_state(T& value);
    template<typename T> void dec_state(T& value);

public:
    Thread_pool(int size);
    ~Thread_pool();

    int get_num_threads() const { return (int) threads.size(); }
    int get_num_active_threads() const { return num_active_threads; }
    int get_num_waiting_threads() const { return num_waiting_threads; }
    int get_num_jobs() const { return num_jobs; }
    int get_num_errors() const { return num_errors; }

    void resize(int num_threads);
    void run(Runnable* target);
};

template<typename T>
void Thread_pool::inc_state(T& value)
{
    Lock::Block lock(mutex);
    value++;
}

template<typename T>
void Thread_pool::dec_state(T& value)
{
    Lock::Block lock(mutex);
    value--;
}

}}

#endif
