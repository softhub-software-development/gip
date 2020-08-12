
//
//  base_serialization.h
//
//  Created by Softhub.
//  Copyright (c) 2019 Softhub. All rights reserved.
//

#ifndef BASE_SERIALIZATION_H
#define BASE_SERIALIZATION_H

#include "base_stl_wrapper.h"
#include "base_exception.h"
#include "base_handle.h"
#include "base_io.h"
#include <stdio.h>
#include <iostream>

#ifdef __GNUC__

namespace SOFTHUB {
namespace BASE {
    class Serializable;
}}

#endif

namespace SOFTHUB {
namespace BASE {

class Class_registry;
class IArray;
class IDictionary;

static const int magic_number = 'shub';

//
// class Serializer
//

class Serializer {

    typedef Hash_map<const Serializable*,class_id_type> Serializable_map;

    int ref_id_counter;
    Serializable_map refs;

protected:
    void write_header();
    void write_array(const IArray* obj);
    void write_object(const Serializable* obj);

    virtual void write_size(unsigned size) { write(size); }

public:
    Serializer();
    virtual ~Serializer() {}

    virtual large tell() const = 0;
    virtual void seek(large pos) = 0;

    virtual void write(bool val) = 0;
    virtual void write(char val) = 0;
    virtual void write(unsigned char val) = 0;
    virtual void write(short val) = 0;
    virtual void write(unsigned short val) = 0;
    virtual void write(int val) = 0;
    virtual void write(unsigned int val) = 0;
    virtual void write(long val) = 0;
    virtual void write(unsigned long val) = 0;
    virtual void write(large val) = 0;
    virtual void write(ularge val) = 0;
    virtual void write(float val) = 0;
    virtual void write(double val) = 0;
    virtual void write(quad val) = 0;
    virtual void write(char* val, int len) = 0;
    virtual void write(const std::string& val) = 0;
    virtual void write(const std::wstring& val) = 0;
    virtual void write(const Serializable* obj, class_id_type cid);

    template <typename T> void write(const Reference<T>& ref);
    template <typename T> void write(const Weak_reference<T>& ref);
    template <typename T> void write(const Handle<T>& handle);
    template <typename T> void write(T* obj) { write(obj, T::class_id); }
};

//
// class Deserializer
//

class Deserializer {

    typedef Hash_map<class_id_type,Serializable*> Serializable_map;

    int id_counter;
    Serializable_map refs;
    version_type version;
    Class_registry* registry;

protected:
    void read_header();
    void read_array(Serializable*& obj);
    void read_object(Serializable*& obj, class_id_type class_id);

    virtual void read_size(unsigned& size) { read(size); }

public:
    Deserializer();
    virtual ~Deserializer();

    virtual large tell() const = 0;
    virtual void seek(large pos) = 0;
    version_type class_version_of(Serializable* obj) const;

    virtual void read(bool& val) = 0;
    virtual void read(char& val) = 0;
    virtual void read(unsigned char& val) = 0;
    virtual void read(short& val) = 0;
    virtual void read(unsigned short& val) = 0;
    virtual void read(int& val) = 0;
    virtual void read(unsigned int& val) = 0;
    virtual void read(long& val) = 0;
    virtual void read(unsigned long& val) = 0;
    virtual void read(large& val) = 0;
    virtual void read(ularge& val) = 0;
    virtual void read(float& val) = 0;
    virtual void read(double& val) = 0;
    virtual void read(quad& val) = 0;
    virtual void read(char* val, int len) = 0;
    virtual void read(std::string& val) = 0;
    virtual void read(std::wstring& val) = 0;
    virtual void read(Serializable*& obj, class_id_type cid);
    virtual void debug_print(std::ostream& stream) = 0;

#ifdef UNIQUE_ID_SUPPORT
    virtual Serializable* find_by_object_id(const Object_id& oid) { return 0; }
#endif

    template <typename T> void read(Reference<T>& ref);
    template <typename T> void read(Weak_reference<T>& ref);
    template <typename T> void read(Handle<T>& handle);
};

//
// class Abstract_serializer
//

class Abstract_serializer : public Serializer {

public:
    void write(long val);
    void write(unsigned long val);
    void write(large val);
    void write(ularge val);
    void write(float val);
    void write(double val);
    void write(quad val);
    void write(const std::string& val);
    void write(const std::wstring& val);
    void write(const Serializable* obj, class_id_type cid) { Serializer::write(obj, cid); }

    using Serializer::write;
};

//
// class Abstract_deserializer
//

class Abstract_deserializer : public Deserializer {

    char buffer[8];

public:
    void read(long& val);
    void read(unsigned long& val);
    void read(large& val);
    void read(ularge& val);
    void read(float& val);
    void read(double& val);
    void read(quad& val);
    void read(std::string& val);
    void read(std::wstring& val);
    void read(Serializable*& obj, class_id_type cid) { Deserializer::read(obj, cid); }
    void debug_print(std::ostream& stream);

    using Deserializer::read;
};

//
// class Stream_serializer
//

class Stream_serializer : public Abstract_serializer {

protected:
    Stream_io_base_ref io;

    Stream_serializer(Stream_io_base* io) : io(io) {}

    void init();

public:
    Stream_serializer(std::ostream& stream);
    Stream_serializer(FILE* file);
    ~Stream_serializer();

    large tell() const { return io->tell(); }
    void seek(large pos) { io->seek((std::ostream::streampos) pos); }

    void write(bool val);
    void write(char val);
    void write(unsigned char val);
    void write(short val);
    void write(unsigned short val);
    void write(int val);
    void write(unsigned int val);
    void write(char* val, int len);

    template <typename T> void write(const Reference<T>& ref) { Serializer::write(ref); }
    template <typename T> void write(T* obj) { Serializer::write(obj, T::class_id); }

    using Abstract_serializer::write;
};

//
// class Stream_deserializer
//

class Stream_deserializer : public Abstract_deserializer {

protected:
    char buffer[8];
    Stream_io_base_ref io;

    Stream_deserializer(Stream_io_base* io) : io(io) {}

    void init();

public:
    Stream_deserializer(std::istream& stream);
    Stream_deserializer(FILE* file);
    ~Stream_deserializer();

    large tell() const { return io->tell(); }
    void seek(large pos) { io->seek((std::ostream::streampos) pos); }

    void read(bool& val);
    void read(char& val);
    void read(unsigned char& val);
    void read(short& val);
    void read(unsigned short& val);
    void read(int& val);
    void read(unsigned int& val);
    void read(char* val, int len);

    template <typename T> void read(Reference<T>& ref) { Deserializer::read(ref); }

    using Abstract_deserializer::read;
};

//
// class Serialization_exception
//

class Serialization_exception : public Exception {

    class_id_type id;
    int err;

public:
    Serialization_exception(const std::string& msg, class_id_type id = 0) : Exception(msg), id(id), err(errno) {}

    std::string get_message() const;
    class_id_type get_id() const { return id; }
    std::string get_id_string_value() const;
};

}}

#include "base_serialization_inline.h"

#endif
