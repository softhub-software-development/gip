
//
//  util_file.h
//
//  Created by Christian Lehner on 02/01/2020.
//  Copyright (c) 2020 Softhub. All rights reserved.
//

#ifndef LIB_UTIL_FILE_H
#define LIB_UTIL_FILE_H

#include <base/base.h>
#include <hal/hal.h>
#include <fstream>

namespace SOFTHUB {
namespace UTIL {

FORWARD_CLASS(File_observer);
FORWARD_CLASS(IFile_consumer);

//
// class File_observer
//

class File_observer : public BASE::Object<> {

    HAL::Mutex mutex;
    HAL::Condition condition;
    std::ifstream::pos_type stream_pos;
    ularge stream_ino;
    IFile_consumer_ref consumer;

    bool process_log_element(std::istream& stream, std::string& str);
    bool process_log_bracketed(std::istream& stream, std::string& str);
    bool process_log_quoted(std::istream& stream, std::string& str);
    bool process_log_line(const std::string& line);
    bool process_log_file(std::istream& stream);
    bool process_tail(const std::string& filepath);
    bool process_tail_lines(const std::string& filepath);

public:
    File_observer(IFile_consumer* consumer);

    bool tail(const std::string& filepath, bool listen = false);
    void refresh();
};

//
// class File_observer
//

class IFile_consumer : public BASE::Interface {

public:
    virtual bool consumer_process(const BASE::String_vector& cols) = 0;
    virtual void consumer_reset() = 0;
};

}}

#endif
