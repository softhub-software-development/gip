
//
//  base_stream_serialization.cpp
//
//  Created by Softhub.
//  Copyright (c) 2019 Softhub. All rights reserved.
//

#include "stdafx.h"
#include "base_serialization.h"
#ifdef PLATFORM_WIN
#include <winsock2.h>
#else
#include <arpa/inet.h>
#define u_short __uint16_t
#define u_long __uint32_t
#endif

using namespace std;

namespace SOFTHUB {
namespace BASE {

//
// class Stream_serializer
//

Stream_serializer::Stream_serializer(ostream& stream) : io(new Stream_io<ostream>(stream))
{
    init();
}

Stream_serializer::Stream_serializer(FILE* file) : io(new Stream_io<FILE>(file))
{
    init();
}

Stream_serializer::~Stream_serializer()
{
}

void Stream_serializer::init()
{
    if (io->is_valid())
        write_header();
}

void Stream_serializer::write(bool val)
{
    char cval = val ? 1 : 0;
    io->write(&cval, 1);
}

void Stream_serializer::write(char val)
{
    io->write(&val, 1);
}

void Stream_serializer::write(unsigned char val)
{
    io->write((const char*) &val, 1);
}

void Stream_serializer::write(short val)
{
    short sval = htons(val);
    char* p = reinterpret_cast<char*>(&sval);
    io->write(p, 2);
}

void Stream_serializer::write(unsigned short val)
{
    unsigned short sval = htons(val);
    char* p = reinterpret_cast<char*>(&sval);
    io->write(p, 2);
}

void Stream_serializer::write(int val)
{
    int lval = htonl(val);
    char* p = reinterpret_cast<char*>(&lval);
    io->write(p, 4);
}

void Stream_serializer::write(unsigned int val)
{
    unsigned int lval = htonl(val);
    char* p = reinterpret_cast<char*>(&lval);
    io->write(p, 4);
}

void Stream_serializer::write(char* val, int len)
{
    io->write(val, len);
}

//
// class Stream_deserializer
//

Stream_deserializer::Stream_deserializer(istream& stream) : io(new Stream_io<istream>(stream))
{
    init();
}

Stream_deserializer::Stream_deserializer(FILE* file) : io(new Stream_io<FILE>(file))
{
    init();
}

Stream_deserializer::~Stream_deserializer()
{
}

void Stream_deserializer::init()
{
    memset(buffer, 0, sizeof(buffer));
    if (io->is_valid())
        read_header();
}

void Stream_deserializer::read(bool& val)
{
    io->read(buffer, 1);
    val = buffer[0] != 0;
}

void Stream_deserializer::read(char& val)
{
    io->read(buffer, 1);
    val = buffer[0];
}

void Stream_deserializer::read(unsigned char& val)
{
    io->read(buffer, 1);
    val = buffer[0];
}

void Stream_deserializer::read(short& val)
{
    io->read(buffer, 2);
    val = ntohs(*((u_short*) buffer));
}

void Stream_deserializer::read(unsigned short& val)
{
    io->read(buffer, 2);
    val = ntohs(*((u_short*) buffer));
}

void Stream_deserializer::read(int& val)
{
    size_t n = io->read(buffer, 4);
    if (n != 4)
        throw Serialization_exception("deserializing invalid class", 0);
    val = ntohl(*((u_long*) buffer));
}

void Stream_deserializer::read(unsigned int& val)
{
    io->read(buffer, 4);
    val = ntohl(*((u_long*) buffer));
}

void Stream_deserializer::read(char* val, int len)
{
    io->read(val, len);
}

}}
