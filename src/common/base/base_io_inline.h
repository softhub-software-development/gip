
//
//  base_io_inline.h
//
//  Copyright (c) 2019 Softhub. All rights reserved.
//

namespace SOFTHUB {
namespace BASE {

//
// class Stream_io
//

template <typename T>
Stream_io<T>::Stream_io(T& stream) : stream(stream)
{
}

template <typename T>
bool Stream_io<T>::is_valid() const
{
    return stream.good();
}

template <typename T>
large Stream_io<T>::tell() const
{
    return stream.tellg();
}

template <typename T>
void Stream_io<T>::seek(large pos)
{
    stream.seekg((std::streampos) pos);
}

template <typename T>
bool Stream_io<T>::read(char& c)
{
    stream.get(c);
    return c != '\0';
}

template <typename T>
void Stream_io<T>::write(char c)
{
    stream.write(&c, 1);
}

template <typename T>
size_t Stream_io<T>::read(char* buf, size_t len)
{
    return stream.readsome(buf, len);
}

template <typename T>
void Stream_io<T>::write(const char* buf, size_t len)
{
    stream.write(buf, len);
}

}}
