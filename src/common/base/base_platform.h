
//
//  base_platform.h
//
//  Created by Christian Lehner on 7/5/12.
//  Copyright (c) 2019 Softhub. All rights reserved.
//

#ifndef BASE_PLATFORM_H
#define BASE_PLATFORM_H

#include <assert.h>

#ifndef NDEBUG
#ifdef assert
#undef assert
#endif
#define assert(expr) ((expr) ? (void) 0 : (log_assert(__FILE__, __LINE__, __FUNCTION__), abort()))
#endif

#ifdef __APPLE__
#define PLATFORM_APPLE
#include <TargetConditionals.h>
#if TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR
#include "base_platform_ios.h"
#elif TARGET_OS_MAC
#include "base_platform_mac.h"
#else
#error unknown apple platform
#endif
#elif defined(__GNUC__)
#include "base_platform_linux.h"
#elif defined(_MSC_VER)
#include "base_platform_win.h"
#else
#error unknown platform
#endif

#ifdef _DEBUG
#define DEBUG_STATE true
#else
#define DEBUG_STATE false
#endif

#define CPLUSPLUS_11 (__cplusplus >= 199711L)
#define CPLUSPLUS_14 (__cplusplus >= 201402L)

typedef enum { ALL, WARN, INFO, ERR } Log_level;

void log_message(Log_level level, const char* msg);
void log_assert(const char* filename, int lineno, const char* func);

#if MEM_LEAK_CHECK

extern "C" {

void dbg_init();
void dbg_dispose();
void* dbg_malloc(size_t n);
void* dbg_realloc(void* ptr, size_t n);
void dbg_free(void* ptr);
char* dbg_strdup(const char* s);
char* dbg_strndup(const char* s, size_t n);
void dbg_statistics();
int dbg_leak_count();

}

#define malloc(n) dbg_malloc(n)
#define realloc(ptr, n) dbg_realloc(ptr, n)
#define free(ptr) dbg_free(ptr)
#define strdup(s) dbg_strdup(s)
#define strdnup(s, n) dbg_strdup(s, n)

#endif
#endif
