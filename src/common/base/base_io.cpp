
//
//  base_io.cpp
//
//  Copyright (c) 2019 Softhub. All rights reserved.
//

#include "stdafx.h"
#include "base_io.h"

using namespace std;

namespace SOFTHUB {
namespace BASE {

//
// specialization class Stream_io<istream>
//

Stream_io<istream>::Stream_io(istream& stream) : stream(stream)
{
}

bool Stream_io<istream>::is_valid() const
{
    return stream.good();
}

large Stream_io<istream>::tell() const
{
    return stream.tellg();
}

void Stream_io<istream>::seek(large pos)
{
    stream.seekg((istream::pos_type) pos);
}

bool Stream_io<istream>::read(char& c)
{
    stream.get(c);
    return c != '\0';
}

void Stream_io<istream>::write(char c)
{
    assert(!"invalid io operation");
}

size_t Stream_io<istream>::read(char* buf, size_t len)
{
    stream.read(buf, len);
    return len;
}

size_t Stream_io<istream>::read_ooo(char* buf, size_t len)
{
    stream.read(buf, len);
    return len;
}

void Stream_io<istream>::write(const char* buf, size_t len)
{
    assert(!"write: invalid io operation");
}

void Stream_io<istream>::write_ooo(const char* buf, size_t len)
{
    assert(!"write_ooo: invalid io operation");
}

//
// specialization class Stream_io<istream>
//

Stream_io<ostream>::Stream_io(ostream& stream) : stream(stream)
{
}

bool Stream_io<ostream>::is_valid() const
{
    return stream.good();
}

large Stream_io<ostream>::tell() const
{
    return stream.tellp();
}

void Stream_io<ostream>::seek(large pos)
{
    stream.seekp((istream::pos_type) pos);
}

bool Stream_io<ostream>::read(char& c)
{
    assert(!"invalid io operation");
    return false;
}

void Stream_io<ostream>::write(char c)
{
    stream.write(&c, 1);
}

size_t Stream_io<ostream>::read(char* buf, size_t len)
{
    assert(!"invalid io operation");
    return 0;
}

size_t Stream_io<ostream>::read_ooo(char* buf, size_t len)
{
    assert(!"invalid io operation");
    return 0;
}

void Stream_io<ostream>::write(const char* buf, size_t len)
{
    stream.write(buf, len);
}

void Stream_io<ostream>::write_ooo(const char* buf, size_t len)
{
    stream.write(buf, len);
}

//
// specialization class Stream_io<FILE>
//

Stream_io<FILE>::Stream_io(FILE* file) : file(file)
{
}

bool Stream_io<FILE>::is_valid() const
{
    return file != 0;
}

large Stream_io<FILE>::tell() const
{
    return ftell(file);
}

void Stream_io<FILE>::seek(large pos)
{
    fseek(file, (long) pos, SEEK_SET);
}

bool Stream_io<FILE>::read(char& c)
{
    char buf[1];
    size_t n = fread(buf, sizeof(bool), 1, file);
    c = buf[0];
    return n != 0;
}

void Stream_io<FILE>::write(char c)
{
    fputc(c, file);
}

size_t Stream_io<FILE>::read(char* buf, size_t len)
{
    return fread(buf, sizeof(char), len, file);
}

size_t Stream_io<FILE>::read_ooo(char* buf, size_t len)
{
    return fread(buf, sizeof(char), len, file);
}

void Stream_io<FILE>::write(const char* buf, size_t len)
{
    fwrite(buf, 1, len, file);
}

void Stream_io<FILE>::write_ooo(const char* buf, size_t len)
{
    fwrite(buf, 1, len, file);
}

}}
