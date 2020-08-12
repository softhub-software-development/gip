
//
//  util_serialization.h
//
//  Created by Christian Lehner on 06/05/15.
//  Copyright (c) 2019 Softhub. All rights reserved.
//

#ifndef SOFTHUB_LIB_UTIL_SERIALIZATION_H
#define SOFTHUB_LIB_UTIL_SERIALIZATION_H

#include "util_fs.h"

namespace SOFTHUB {
namespace UTIL {

//
// class Directory_serializer
//

class Directory_serializer : public BASE::Serializer {

    File_storage* storage;

    static void write_content(const BASE::Serializable* obj, const std::string& filepath);

public:
    Directory_serializer();

    bool is_valid() const { return true; }
    large tell() const { return 0; }
    void seek(large pos) {}

    void write(bool val) {}
    void write(char val) {}
    void write(unsigned char val) {}
    void write(short val) {}
    void write(unsigned short val) {}
    void write(int val) {}
    void write(unsigned int val) {}
    void write(long val) {}
    void write(unsigned long val) {}
    void write(large val) {}
    void write(ularge val) {}
    void write(float val) {}
    void write(double val) {}
    void write(quad val) {}
    void write(char* val, int len) {}
    void write(const std::string& val) {}
    void write(const std::wstring& val) {}
    void write(const BASE::Serializable* obj, class_id_type cid);
    template <typename T> void write(const T* obj);

    template <typename T> void write(const BASE::Reference<T>& ref) { Serializer::write(ref); }
};

//
// class Directory_deserializer
//

class Directory_deserializer : public BASE::Deserializer {

    File_storage* storage;

public:
    Directory_deserializer();

    bool is_valid() const { return true; }
    large tell() const { return 0; }
    void seek(large pos) {}

    void read(bool& val) {}
    void read(char& val) {}
    void read(unsigned char& val) {}
    void read(short& val) {}
    void read(unsigned short& val) {}
    void read(int& val) {}
    void read(unsigned int& val) {}
    void read(long& val) {}
    void read(unsigned long& val) {}
    void read(large& val) {}
    void read(ularge& val) {}
    void read(float& val) {}
    void read(double& val) {}
    void read(quad& val) {}
    void read(char* val, int len) {}
    void read(std::string& val) {}
    void read(std::wstring& val) {}
    void read(BASE::Serializable*& obj, class_id_type cid) { Deserializer::read(obj, cid); }
    template <typename T> void read(T*& obj);

    template <typename T> void read(BASE::Reference<T>& ref) { Deserializer::read(ref); }
};


//
// class Directory_serializer
//

template <typename T>
void Directory_serializer::write(const T* obj)
{
    storage->write(obj);
}

//
// class Directory_deserializer
//

template <typename T>
void Directory_deserializer::read(T*& obj)
{
    storage->read(obj);
}

}}

#endif
