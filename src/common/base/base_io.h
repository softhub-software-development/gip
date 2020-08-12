
//
//  base_io.h
//
//  Created by Softhub.
//  Copyright (c) 2019 Softhub. All rights reserved.
//

#ifndef BASE_IO_H
#define BASE_IO_H

#include "base_reference.h"
#include <stdio.h>
#include <iostream>

namespace SOFTHUB {
namespace BASE {

FORWARD_CLASS(Stream_io_base);

//
// class Stream_io_base
//

class Stream_io_base : public Object<> {

public:
    virtual bool is_valid() const = 0;
    virtual large tell() const = 0;
    virtual void seek(large pos) = 0;
    virtual bool read(char& c) = 0;
    virtual void write(char c) = 0;
    virtual size_t read(char* buf, size_t len) = 0;
    virtual size_t read_ooo(char* buf, size_t len) = 0;
    virtual void write(const char* buf, size_t len) = 0;
    virtual void write_ooo(const char* buf, size_t len) = 0;
};

//
// class Stream_io
//

template <typename T = std::iostream>
class Stream_io : public Stream_io_base {

    T& stream;

public:
    Stream_io(T& stream);

    bool is_valid() const;
    large tell() const;
    void seek(large pos);
    bool read(char& c);
    void write(char c);
    size_t read(char* buf, size_t len);
    size_t read_ooo(char* buf, size_t len);
    void write(const char* buf, size_t len);
    void write_ooo(const char* buf, size_t len);
};

//
// class Stream_io
//

template <>
class Stream_io<std::istream> : public Stream_io_base {

    std::istream& stream;

public:
    Stream_io(std::istream& stream);

    bool is_valid() const;
    large tell() const;
    void seek(large pos);
    bool read(char& c);
    void write(char c);
    size_t read(char* buf, size_t len);
    size_t read_ooo(char* buf, size_t len);
    void write(const char* buf, size_t len);
    void write_ooo(const char* buf, size_t len);
};

//
// class Stream_io
//

template <>
class Stream_io<std::ostream> : public Stream_io_base {

    std::ostream& stream;

public:
    Stream_io(std::ostream& stream);

    bool is_valid() const;
    large tell() const;
    void seek(large pos);
    bool read(char& c);
    void write(char c);
    size_t read(char* buf, size_t len);
    size_t read_ooo(char* buf, size_t len);
    void write(const char* buf, size_t len);
    void write_ooo(const char* buf, size_t len);
};

//
// class Stream_io specialication for FILE
//

template <>
class Stream_io<FILE> : public Stream_io_base {

    FILE* file;

public:
    Stream_io(FILE* file);

    bool is_valid() const;
    large tell() const;
    void seek(large pos);
    bool read(char& c);
    void write(char c);
    size_t read_ooo(char* buf, size_t len);
    size_t read(char* buf, size_t len);
    void write_ooo(const char* buf, size_t len);
    void write(const char* buf, size_t len);
};

}}

#include "base_io_inline.h"

#endif
