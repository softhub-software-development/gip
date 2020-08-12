
//
//  base_platform_win.h
//
//  Created by Christian Lehner on 11/8/15.
//  Copyright (c) 2019 Softhub. All rights reserved.
//

#ifndef BASE_PLATFORM_WIN_H
#define BASE_PLATFORM_WIN_H

#include <time.h>
#include <iomanip>
#include <sstream>
#include <ctime>
#include <io.h>

#define PLATFORM_WIN
#define TOOLCHAIN_MSVC
#define WINDOWS_XP_COMPATIBLE

#ifndef WIN32
#define WIN32
#endif

#ifdef ABSOLUTE
#undef ABSOLUTE
#endif

#define STDERR_FILENO   2

#define isatty(x) _isatty(x)
#define fileno(x) _fileno(x)

typedef int mode_t;
typedef unsigned short ushort;

inline char* strptime(const char* s, const char* f, struct tm* tm)
{
    std::istringstream input(s);
    input.imbue(std::locale(setlocale(LC_ALL, nullptr)));
    input >> std::get_time(tm, f);
    return input.fail() ? nullptr : (char*) (s + (int) input.tellg());
}

#endif
