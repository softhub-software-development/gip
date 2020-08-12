
//
//  hal_time.cpp
//
//  Created by Christian Lehner on 17/04/16.
//  Copyright (c) 2019 Softhub. All rights reserved.
//

#include "hal_time.h"
#ifdef PLATFORM_WIN
#include "hal_strings.h"
#include <windows.h>
#include <winbase.h>
#include <shlwapi.h>
#include <shellapi.h>
#include <tchar.h>
#include "NB30.H"
#include <intrin.h>
#include <iphlpapi.h>
#define PACKVERSION(major, minor) MAKELONG(minor, major)
#define PLATFORM_ID_SIZE (4 * sizeof(int))
#elif defined PLATFORM_MAC
#include <IOKit/IOKitLib.h>
#elif defined PLATFORM_LINUX
#include <sys/time.h>
#include <unistd.h>
#include <iostream>
#include <sys/utsname.h>
#elif defined PLATFORM_IOS
#else
#error undefined platform
#endif
#include <time.h>
#ifdef PLATFORM_MAC
#include <libproc.h>
#include <stdio.h>
#include <string.h>
#endif

#define TIME_BOMB_YEAR 2022
#define TIME_BOMB_MONTH 04
#define TIME_BOMB_DAY 22

using namespace std;

namespace SOFTHUB {
namespace HAL {

void time_bomb()
{
    struct tm t;
    ::memset(&t, 0, sizeof(t));
    t.tm_year = TIME_BOMB_YEAR - 1900;
    t.tm_mon = TIME_BOMB_MONTH - 1;
    t.tm_mday = TIME_BOMB_DAY;
    t.tm_hour = 12 - 1;
    time_t then = mktime(&t);
    time_t now = time(0);
    if (TIME_BOMB_ACTIVE && now > then) {
#ifdef PLATFORM_WIN
        MessageBox(0, _T("Please update this trial version from 'http://www.softhub.com'."), _T("Expiration"), MB_ICONERROR);
        ShellExecute(NULL, _T("open"), _T("http://www.softhub.com/bounty"), NULL, NULL, SW_SHOWNORMAL);
        char* s = ctime(&then);
        log_message(INFO, s);
        exit(0);
#else
        // TODO: show message
#endif
    }
}

//
// class Timing
//

Timing::Timing() : msec(0)
{
#ifdef PLATFORM_WIN
    start.QuadPart = 0;
    stop.QuadPart = 0;
#else
    start.tv_sec = 0;
    start.tv_usec = 0;
    stop.tv_sec = 0;
    stop.tv_usec = 0;
#endif
}

void Timing::begin() const
{
#ifdef PLATFORM_WIN
    QueryPerformanceCounter(&start);
#else
    gettimeofday(&start, 0);
#endif
}

long Timing::end() const
{
#ifdef PLATFORM_WIN
    QueryPerformanceCounter(&stop);
    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);
    return (long) ((stop.QuadPart - start.QuadPart) * 1000 / frequency.QuadPart);
#else
    gettimeofday(&stop, 0);
    return (stop.tv_sec - start.tv_sec) * 1000 + (stop.tv_usec - start.tv_usec) / 1000;
#endif
}

//
// class Timing::Block
//

Timing::Block::Block(const std::string& label, const Timing& timing) :
    label(label), timing(timing)
{
    timing.begin();
}

Timing::Block::~Block()
{
    print(timing.end());
}

void Timing::Block::print(long t)
{
    stringstream stream;
    stream << label << ": " << t << "ms" << endl;
    string s = stream.str();
#ifdef PLATFORM_WIN
    OutputDebugStringA(s.c_str());
#else
    cout << s;
#endif
}

}}
