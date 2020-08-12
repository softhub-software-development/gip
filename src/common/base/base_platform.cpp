
//
//  base_platform.cpp
//
//  Created by Christian Lehner on 28/05/16.
//  Copyright (c) 2019 Softhub. All rights reserved.
//

#include "stdafx.h"
#include "base_platform.h"
#include <iostream>
#if MEM_LEAK_CHECK
#include "base_stl_wrapper.h"
#include <iostream>
#include <sstream>
#endif

namespace SOFTHUB {
namespace BASE {

}}

#if MEM_LEAK_CHECK

#include <hal/hal_lock.h>

using namespace SOFTHUB::HAL;
using namespace std;

#undef malloc
#undef free
#undef strdup

struct dbg_allocation {

    int id;
    size_t size;
};

namespace std {

template<> struct hash<dbg_allocation*> {
    size_t operator()(dbg_allocation* val) const
    {
        return std::hash<int>()((int) reinterpret_cast<size_t>(val));
    }
};

}

typedef SOFTHUB::BASE::Hash_set<dbg_allocation*> Allocations;

static int alloc_count;
static int free_count;
static Mutex dbg_mutex;
static bool in_dbg_alloc;
static Allocations* allocations;

#ifndef DBG_STORE_POINTERS
#define DBG_STORE_POINTERS 1
#endif
#ifndef DBG_OVERLOAD_GLOBAL_ALLOCATOR
#define DBG_OVERLOAD_GLOBAL_ALLOCATOR 1
#endif

void dbg_init()
{
    alloc_count = free_count = 0;
    in_dbg_alloc = true;
    if (allocations)
        dbg_dispose();
    allocations = new Allocations();
    in_dbg_alloc = false;
}

void dbg_dispose()
{
    allocations->clear();
    delete allocations;
    allocations = 0;
}

void* dbg_malloc(size_t n)
{
    Lock::Block lock(dbg_mutex);
    dbg_allocation* data = (dbg_allocation*) ::malloc(n + sizeof(dbg_allocation));
    if (!data)
        return 0;
    data->id = alloc_count++;
    data->size = n;
    void* ptr = (void*) (((char*) data) + sizeof(dbg_allocation));
#if DBG_STORE_POINTERS
    if (allocations) {
        in_dbg_alloc = true;
        allocations->insert(data);
        in_dbg_alloc = false;
    }
#endif
    return ptr;
}

void* dbg_realloc(void* ptr, size_t n)
{
    void* new_ptr;
    if (ptr) {
        Lock::Block lock(dbg_mutex);
        dbg_allocation* old_data = (dbg_allocation*) (((char*) ptr) - sizeof(dbg_allocation));
        dbg_allocation* new_data = (dbg_allocation*) ::malloc(n + sizeof(dbg_allocation));
        ::memcpy(new_data, old_data, old_data->size + sizeof(dbg_allocation));
        new_ptr = (void*) (((char*) new_data) + sizeof(dbg_allocation));
#if DBG_STORE_POINTERS
        if (allocations) {
            in_dbg_alloc = true;
            allocations->insert(new_data);
            in_dbg_alloc = false;
        }
#endif
    } else {
        new_ptr = dbg_malloc(n);
    }
    return new_ptr;
}

void dbg_free(void* ptr)
{
    if (!ptr)
        return;
    Lock::Block lock(dbg_mutex);
    dbg_allocation* data = (dbg_allocation*) (((char*) ptr) - sizeof(dbg_allocation));
#if DBG_STORE_POINTERS
    if (allocations) {
        in_dbg_alloc = true;
        bool flag = allocations->remove(data);
        if (flag)
            ::free(data);
        in_dbg_alloc = false;
    }
#else
    ::free(data);
#endif
    free_count++;
}

char* dbg_strdup(const char* s)
{
    if (s == 0)
        return 0;
    size_t slen = strlen(s);
    char* str = dbg_strndup(s, slen + 1);
    str[slen] = '\0';
    return str;
}

char* dbg_strndup(const char* s, size_t n)
{
    if (s == 0)
        return 0;
    if (n == 0)
        return (char*) s;
    Lock::Block lock(dbg_mutex);
    char* str = (char*) dbg_malloc(n);
    strncpy(str, s, n);
    return str;
}

void dbg_print_ptr(ostream& stream, void* ptr, int len)
{
    stream << ptr;
    char* cptr = (char*) ptr;
    for (int i = 0, n = std::min(32, len); i < n; i++) {
        char c = cptr[i];
        char cc = 32 <= c && c <= 127 ? c : '?';
        stream << " '" << cc << "'";//"' " << std::hex << unsigned(c) << std::dec;
    }
}

void dbg_statistics()
{
    if (!allocations)
        return;
    in_dbg_alloc = true;
    {
    string msg;
    stringstream stream;
    int leaks = dbg_leak_count();
    stream << "stored: " << allocations->size() << " allocated: " << alloc_count << " free'd: " << free_count << " leaks: " << leaks << endl;
    msg = stream.str();
    log_message(WARN, msg.c_str());
    stream.str("");
    int count = 0;
    Allocations::const_iterator it = allocations->begin();
    Allocations::const_iterator tail = allocations->end();
    while (it != tail) {
        dbg_allocation* data = *it++;
        void* ptr = (void*) (((char*) data) + sizeof(dbg_allocation));
        stream << count << " id: " << data->id << " size: " << data->size << " ";
        dbg_print_ptr(stream, ptr, (int) data->size);
        stream << endl;
        msg = stream.str();
        log_message(WARN, msg.c_str());
        stream.str("");
        count++;
    }
    cout << endl;
    }
    in_dbg_alloc = false;
}

int dbg_leak_count()
{
    return alloc_count - free_count;
}

#if DBG_OVERLOAD_GLOBAL_ALLOCATOR

void* operator new(size_t n) throw(std::bad_alloc)
{
#if DBG_STORE_POINTERS
    if (!allocations)
        return ::malloc(n);
#endif
    Lock::Block lock(dbg_mutex);
    void* ptr = in_dbg_alloc ? ::malloc(n) : dbg_malloc(n);
    if (!ptr)
        throw std::bad_alloc();
    return ptr;
}

void operator delete(void* ptr) throw()
{
#if DBG_STORE_POINTERS
    if (!allocations)
        ::free(ptr);
    else
#endif
    {
    Lock::Block lock(dbg_mutex);
    if (in_dbg_alloc)
        ::free(ptr);
    else
        dbg_free(ptr);
    }
}

#if 0
void* operator new[](size_t n) throw(std::bad_alloc)
{
    void* ptr = dbg_malloc(n);
    if (!ptr)
        throw std::bad_alloc();
    return ptr;
}

void operator delete[](void* ptr) throw()
{
    dbg_free(ptr);
}
#endif
#endif
#endif
