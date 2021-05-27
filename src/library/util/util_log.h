
//
//  util_log.h
//
//  Created by Christian Lehner on 14/05/15.
//  Copyright (c) 2019 Softhub. All rights reserved.
//

#ifndef LIB_UTIL_LOG_H
#define LIB_UTIL_LOG_H

#include <base/base.h>
#include <fstream>

namespace SOFTHUB {
namespace UTIL {

//
// class Logging
//

class Logging {

    static void backup(const std::string& path);

public:
    Logging() {}

    static void init(const std::string& filepath);
    static void finalize();
};

//
// class Log_stream
//

class Log_stream : public std::ofstream {

    bool enabled;
    bool start_of_line;

    friend Log_stream& endl(Log_stream& stream);

    template <typename T> void cvs(T val);

    void write_header();

public:
    Log_stream() : enabled(true), start_of_line(true) {}

    void set_enabled(bool state) { enabled = state; }
    bool is_enabled() const { return enabled; }

    Log_stream& operator<<(Log_stream& (*fun)(Log_stream& param));
    Log_stream& operator<<(bool val);
    Log_stream& operator<<(short val);
    Log_stream& operator<<(unsigned short val);
    Log_stream& operator<<(int val);
    Log_stream& operator<<(unsigned int val);
    Log_stream& operator<<(long val);
    Log_stream& operator<<(unsigned long val);
    Log_stream& operator<<(large val);
    Log_stream& operator<<(float val);
    Log_stream& operator<<(double val);
    Log_stream& operator<<(long double val);
    Log_stream& operator<<(const void* val);
    Log_stream& put(char_type c);
    Log_stream& write(const char_type* s, std::streamsize n);
    Log_stream& flush();
};

inline Log_stream& operator<<(Log_stream& stream, const std::string& str)
{
    return stream.write(str.c_str(), str.length());
}

inline Log_stream& operator<<(Log_stream& stream, const char* str)
{
    return stream.write(str, strlen(str));
}

inline Log_stream& endl(Log_stream& stream)
{
    stream.put(stream.widen('\n'));
    stream.start_of_line = true;
    stream.flush();
    return stream;
}

template <typename T>
void Log_stream::cvs(T val)
{
    std::stringstream ss;
    ss << val;
    std::string s = ss.str();
    write(s.c_str(), s.length());
}

}

extern UTIL::Log_stream clog;
extern UTIL::Log_stream cdbg;
extern UTIL::Log_stream cacc;

void debug(const std::string& msg);

}

#endif
