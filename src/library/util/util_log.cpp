
//
//  util_log.cpp
//
//  Created by Christian Lehner on 14/05/15.
//  Copyright (c) 2019 Softhub. All rights reserved.
//

#include <stdafx.h>
#include "util_log.h"
#include <base/base.h>
#include <hal/hal.h>
#include <sstream>
#ifdef PLATFORM_WIN
#include <windows.h>
#include <time.h>
#include <io.h>
#define dup2 _dup2
#else
#include <unistd.h>
#endif

#define USE_OSTREAM 0

#if USE_OSTREAM
#define CONVERT(val) ostream::operator<<(val)
#else
#define CONVERT(val) cvs(val)
#endif

using namespace SOFTHUB::HAL;
using namespace std;

namespace SOFTHUB {
namespace UTIL {

static const char* info_file = "info.log";
static const char* dbg_file = "debug.log";
static const char* error_file = "error.log";
static const char* access_file = "access.log";

//
// class Logging
//

void Logging::init(const string& filepath)
{
    time_t now = time(0);
    const char* date = ctime(&now);
    File_path::ensure_dir(filepath, 0755);
    string info_log = filepath + info_file;
    backup(info_log);
    clog.open(info_log.c_str(), fstream::out);
    if (!clog.fail())
        clog << "softhub log created on " << date << endl;
    string dbg_log = filepath + dbg_file;
    backup(dbg_log);
    cdbg.open(dbg_log.c_str(), fstream::out);
    if (!cdbg.fail())
        cdbg << "softhub debug log created on " << date << endl;
    string error_log = filepath + error_file;
    backup(error_log);
    FILE* err_log = fopen(error_log.c_str(), "w");
    if (err_log) {
        // redirect stderr to error.log
#ifdef PLATFORM_WIN
        int err_log_no = _fileno(err_log);
#else
        int err_log_no = fileno(err_log);
#endif
        dup2(err_log_no, STDERR_FILENO);
    }
    string access_log = filepath + access_file;
    backup(access_log);
    cacc.open(access_log.c_str(), fstream::out);
    if (!cacc.fail())
        cacc << "softhub access log created on " << date << endl;
}

void Logging::finalize()
{
    time_t now = time(0);
    const char* date = ctime(&now);
    cacc << "softhub access log closed on " << date << endl;
    clog.close();
}

void Logging::backup(const string& path)
{
    const char* s = path.c_str();
    FILE* file = fopen(s, "r");
    if (file) {
        string bak = path + ".1";
        rename(s, bak.c_str());
    }
}

//
// class Log_stream
//

void Log_stream::write_header()
{
    char buffer[80];
    time_t now = time(0);
    struct tm* timeinfo = localtime(&now);
    strftime(buffer, 80, "[%X] ", timeinfo);
    ofstream::write(buffer, strlen(buffer));
}

Log_stream& Log_stream::operator<<(Log_stream& (*fun)(Log_stream& param))
{
    return fun(*this);
}

Log_stream& Log_stream::operator<<(bool val)
{
    CONVERT(val);
    return *this;
}

Log_stream& Log_stream::operator<<(short val)
{
    CONVERT(val);
    return *this;
}

Log_stream& Log_stream::operator<<(unsigned short val)
{
    CONVERT(val);
    return *this;
}

Log_stream& Log_stream::operator<<(int val)
{
    CONVERT(val);
    return *this;
}

Log_stream& Log_stream::operator<<(unsigned int val)
{
    CONVERT(val);
    return *this;
}

Log_stream& Log_stream::operator<<(long val)
{
    CONVERT(val);
    return *this;
}

Log_stream& Log_stream::operator<<(unsigned long val)
{
    CONVERT(val);
    return *this;
}

Log_stream& Log_stream::operator<<(large val)
{
    CONVERT(val);
    return *this;
}

Log_stream& Log_stream::operator<<(float val)
{
    CONVERT(val);
    return *this;
}

Log_stream& Log_stream::operator<<(double val)
{
    CONVERT(val);
    return *this;
}

Log_stream& Log_stream::operator<<(const void* val)
{
    CONVERT(val);
    return *this;
}

Log_stream& Log_stream::put(char_type c)
{
    if (enabled) {
        if (start_of_line) {
            write_header();
            start_of_line = false;
        }
        ofstream::put(c);
#ifdef _DEBUG
        cout << c;
#endif
    }
    return *this;
}

Log_stream& Log_stream::write(const char_type* s, std::streamsize n)
{
    if (enabled) {
        if (start_of_line) {
            write_header();
            start_of_line = false;
        }
        ofstream::write(s, n);
#ifdef _DEBUG
        cout << s;
#endif
    }
    return *this;
}

Log_stream& Log_stream::flush()
{
    if (enabled)
        ofstream::flush();
    return *this;
}

}

UTIL::Log_stream clog;
UTIL::Log_stream cdbg;
UTIL::Log_stream cacc;

void debug(const string& msg)
{
#ifdef PLATFORM_WIN
    OutputDebugStringA(msg.c_str());
#else
    printf("%s\n", msg.c_str());
#endif
}

}
