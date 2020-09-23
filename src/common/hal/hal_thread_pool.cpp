
#include "stdafx.h"
#include "hal_thread_pool.h"
#if _DEBUG_PERF
#include <sys/time.h>
#endif

namespace SOFTHUB {
namespace HAL {

using namespace BASE;
using namespace std;

//
// class Pool_thread, TODO: needs rewrite
//

class Pool_thread : public Thread_base {

    Thread_pool* pool;
    Runnable_ref target;

    void run();

public:
    Pool_thread(Thread_pool* pool) : pool(pool) {}
    ~Pool_thread() {}

    bool is_busy() { return target; }
    void run(Runnable* target);
    void stop();
};

void Pool_thread::stop()
{
//  Lock::Block lock(mutex);
    Thread_base::stop();
    target = 0;
    condition.signal();
}

void Pool_thread::run(Runnable* target)
{
//  Lock::Block lock(mutex);
    assert(!this->target);
    this->target = target;
    condition.signal();
}

void Pool_thread::run()
{
    Lock::Block lock(mutex);
    while (!is_stopped()) {
        pool->inc_state(pool->num_waiting_threads);
        condition.wait(mutex);
        pool->dec_state(pool->num_waiting_threads);
        if (target) {
#if _DEBUG_PERF >= 10
            Timing timing;
            timing.begin();
#endif
            pool->inc_state(pool->num_active_threads);
            pool->execute(target);
            target = 0;
            pool->dec_state(pool->num_active_threads);
#if _DEBUG_PERF >= 10
            long msec = timing.end();
            stringstream stream;
            stream << "thread runtime " << msec;
            log_message(INFO, stream.str());
#endif
        }
    }
    pool->terminate(this);
}

//
// class Thread_pool
//

Thread_pool::Thread_pool(int size) :
    num_threads(size), num_active_threads(0), num_waiting_threads(0), num_jobs(0), num_errors(0), terminated(false)
{
    reserve_memory = new byte[reserve_memory_size];
    resize(size);
}

Thread_pool::~Thread_pool()
{
    terminated = true;
    // request threads to stop
    for (size_t i = 0, n = threads.size(); i < n; i++)
        threads[i]->stop();
    for (size_t i = 0, n = threads.size(); i < n; i++)
        threads[i]->join();
    // wait for all pool threads to terminate
    int count = 0;
    while ((num_threads > 0 || num_active_threads > 0) && count++ < 10)
        Thread::sleep(1000);
    // delete the pool threads
    for (size_t i = 0, n = threads.size(); i < n; i++)
        delete threads[i];
    if (reserve_memory)
        delete[] reserve_memory;
}

void Thread_pool::terminate(Pool_thread* thread)
{
    Lock::Block lock(mutex);
    num_threads--;
    log_message(INFO, "pool thread terminated");
}

void Thread_pool::last_resort()
{
    Lock::Block lock(mutex);
    if (reserve_memory) {
        delete[] reserve_memory;
        reserve_memory = 0;
        log_message(ERR, "pool thread released reserve memory");
    } else {
        log_message(ERR, "pool thread has no reserve memory");
    }
}

void Thread_pool::execute(Runnable* target)
{
    try {
        target->run();
        inc_state(num_jobs);
    } catch (Exception& ex) {
        inc_state(num_errors);
        stringstream stream;
        stream << "pool thread: " << ex.get_message();
        log_message(ERR, stream.str());
        target->fail(ex);
    } catch (bad_alloc& ex) {
        inc_state(num_errors);
        last_resort();
        log_message(ERR, "pool thread ran out of memory");
        target->fail(ex);
    } catch (...) {
        inc_state(num_errors);
        log_message(ERR, "pool thread failed fatally");
    }
}

void Thread_pool::resize(int num_threads)
{
    Lock::Block lock(mutex);
    int pool_size = (int) threads.size();
    if (num_threads == pool_size)
        return;
    threads.resize(num_threads);
    for (int i = pool_size; i < num_threads; i++) {
        threads[i] = new Pool_thread(this);
        threads[i]->start();
    }
}

void Thread_pool::run(Runnable* target)
{
    Lock::Block lock(mutex);
    for (int i = 0; i < out_of_pool_threads_retries && !terminated; i++) {
        for (size_t j = 0, n = threads.size(); j < n; j++) {
            Pool_thread* thread = threads[j];
            if (!thread->is_busy()) {
                thread->run(target);
                return;
            }
        }
        log_message(WARN, "temporarily out of pool threads");
        condition.wait(mutex, i * 250);
    }
    inc_state(num_errors);
    log_message(WARN, "failed to run thread - out of pool threads");
    throw Exception("out of pool threads");
}

}}
