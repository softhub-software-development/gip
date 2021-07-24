
//
//  util_parser.cpp
//
//  Created by Christian Lehner on 02/01/2020.
//  Copyright (c) 2020 Softhub. All rights reserved.
//

#include "util_file.h"
#include "util_string.h"
#include <hal/hal.h>

using namespace SOFTHUB::BASE;
using namespace SOFTHUB::HAL;
using namespace std;

namespace SOFTHUB {
namespace UTIL {

//
// class File_observer
//

File_observer::File_observer(IFile_consumer* consumer) :
    stream_pos(0), stream_ino(0), consumer(consumer)
{
}

bool File_observer::tail(const string& filepath, bool listen)
{
    bool success = false;
    ifstream stream;
    stream.open(filepath, fstream::in);
    if (stream.good()) {
//      stream.seekg(0, ios::end);
        success = process_log_file(stream);
        stream.close();
        if (success && listen)
            success = process_tail(filepath);
#if _DEBUG
    } else {
        cout << "failed to open " << filepath << endl;
#endif
    }
    return success;
}

void File_observer::refresh()
{
    Lock::Block lock(mutex);
    condition.signal();
}

bool File_observer::process_log_file(istream& stream)
{
    bool success = true;
    string line;
    while (success && !stream.eof() && getline(stream, line)) {
        success = consumer->consumer_process(line);
        stream_pos = stream.tellg();
    }
    return success;
}

bool File_observer::process_tail(const string& filepath)
{
    bool done = false;
    while (!done) {
        Lock::Block lock(mutex);
        condition.wait(mutex, 10000);
        done = process_tail_lines(filepath);
    }
    return true;
}

bool File_observer::process_tail_lines(const string& filepath)
{
    // check if this could be done more elegantly using inotify
    ifstream stream;
    stream.open(filepath, fstream::in);
    if (!stream.good())
        return false;
    bool log_rot = false;
    struct stat st;
    int result = ::stat(filepath.c_str(), &st);
    if (result == 0) {
        log_rot = stream_ino != 0 && st.st_ino != stream_ino;
        stream_ino = st.st_ino;
    }
    if (log_rot) {
        consumer->consumer_reset();
        stream_pos = 0;
    } else {
        string line;
        stream.seekg(stream_pos, ios::beg);
        while (!stream.eof() && getline(stream, line)) {
            consumer->consumer_process(line);
            stream_pos = stream.tellg();
        }
    }
    // TODO: interrupt
    stream.close();
    return false;
}

}}
