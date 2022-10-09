
//
//  base_serialization.cpp
//
//  Created by Softhub.
//  Copyright (c) 2019 Softhub. All rights reserved.
//

#include "stdafx.h"
#include "base_module.h"
#include "base_class_registry.h"
#include "base_serialization.h"
#ifdef PLATFORM_WIN
#include <winsock2.h>
#else
#include <arpa/inet.h>
#define u_short __uint16_t
#define u_long __uint32_t
#endif
#include <cerrno>
#include <cstring>
#include <limits>

#define LENGTH_VALIDATION_BYTE(x) (((x) >> 24) ^ ((x) >> 16) ^ ((x) >> 8) ^ (x))

namespace SOFTHUB {
namespace BASE {

typedef union {
    unsigned a[2];
    ularge val;
} Large_value;

typedef union {
    unsigned a[4];
    quad val;
} Quad_value;

static Serializable_class* lookup_class(class_id_type class_id);

//
// class Serializer
//

Serializer::Serializer() : ref_id_counter(0)
{
}

void Serializer::write_header()
{
    version_type version = 0;
    write(magic_number);
    write(version);
    Class_registry* registry = Base_module::module.instance->get_class_registry();
    assert(registry);
    registry->serialize(this);
}

void Serializer::write_array(const IArray* obj)
{
    class_id_type class_id = obj->get_class_id();
    bool elements_const = obj->is_element_const();
    class_id_type element_class_id = obj->get_element_class_id();
    if (element_class_id == 0)
        throw Serialization_exception("array element class not registered", class_id);
    write(class_id);
    write(element_class_id);
    write(elements_const);
    obj->serialize(this);
}

void Serializer::write_object(const Serializable* obj)
{
    class_id_type class_id = obj->get_class_id();
    assert(class_id);
    write(class_id);
#ifdef UNIQUE_ID_SUPPORT
    const Object_id& oid = obj->get_object_id();
    oid.serialize(this);
#endif
    large base = tell();
    if (base < 0)
        throw Serialization_exception("can't tell base", class_id);
    unsigned size = 0;
    write_size(size);
    obj->serialize(this);
    large end = tell();
    if (end < 0)
        throw Serialization_exception("can't tell end", class_id);
    assert(end >= base);
    size = (unsigned) (end - base);
    seek(base);
    write_size(size);
    seek(end);
}

void Serializer::write(const Serializable* obj, class_id_type cid)     // TODO: check whether we can omit cid
{
    if (obj) {
        Serializable_map::const_iterator it = refs.find(obj);
        if (it == refs.end()) {
            refs[obj] = ++ref_id_counter;
            class_id_type class_id = obj->get_class_id();
            if (class_id > 0) {
                Serializable_class* clazz = lookup_class(class_id);
                if (clazz == 0)
                    throw Serialization_exception("class not registered", class_id);
            }
            if (class_id == class_id_array) {
                const IArray* array = dynamic_cast<const IArray*>(obj);
                assert(array);
                write_array(array);
            } else {
                assert(class_id);
                write_object(obj);
            }
        } else {
            class_id_type ref_id = it->second;
            write(class_id_ref);
            write(ref_id);
        }
    } else {
        write(class_id_nil);
    }
}

//
// class Deserializer
//

Deserializer::Deserializer() :
    id_counter(0), registry(new Class_registry())
{
}

Deserializer::~Deserializer()
{
    delete registry;
}

void Deserializer::read_header()
{
    int magic;
    read(magic);
    if (magic != magic_number)
        throw Serialization_exception("invalid stream", magic);
    read(version);
    registry->deserialize(this);
}

version_type Deserializer::class_version_of(Serializable* obj) const
{
    class_id_type cid = obj->get_class_id();
    Serializable_class* clazz = registry->lookup_class(cid);
    return clazz ? clazz->get_version() : 0;
}

void Deserializer::read_array(Serializable*& obj)
{
    class_id_type element_class_id;
    bool elements_const;
    read(element_class_id);
    read(elements_const);
    Serializable_class* clazz = lookup_class(element_class_id);
    if (clazz == 0)
        throw Serialization_exception("unknown array element class id", element_class_id);
    if (elements_const)
        obj = clazz->create_array_const();
    else
        obj = clazz->create_array();
    obj->retain();
    refs[++id_counter] = obj;
    obj->deserialize(this);
    obj->release_internal();
}

void Deserializer::read_object(Serializable*& obj, class_id_type class_id)
{
#ifdef UNIQUE_ID_SUPPORT
    Object_id oid;
    oid.deserialize(this);
    obj = find_by_object_id(oid);
    if (!obj) {
        Serializable_class* clazz = lookup_class(class_id);
        if (clazz == 0)
            throw Serialization_exception("unknown class id", class_id);
        obj = clazz->create();
        obj->retain();
    }
#else
    Serializable_class* clazz = lookup_class(class_id);
    if (clazz == 0)
        throw Serialization_exception("unknown class id", class_id);
    obj = clazz->create();
    obj->retain();
#endif
    refs[++id_counter] = obj;
    large base = tell();
    if (base < 0)
        throw Serialization_exception("can't tell base", class_id);
    unsigned size;
    read_size(size);
    obj->deserialize(this);
    obj->release_internal();
    large end = tell();
    if (end < 0)
        throw Serialization_exception("can't tell end", class_id);
    unsigned size_deserialized = (unsigned) (end - base);
    int delta = (int) size - (int) size_deserialized;
    if (delta < 0)
        throw Serialization_exception("incorrect size", class_id);
    if (delta > 0)
        seek(end + delta);
}

void Deserializer::read(Serializable*& obj, class_id_type cid)         // TODO: check whether we can omit cid
{
    class_id_type class_id;
    read(class_id);
    if (class_id == class_id_nil) {
        obj = 0;
    } else if (class_id == class_id_ref) {
        class_id_type ref_id;
        read(ref_id);
        obj = refs[ref_id];
        if (!obj)
            throw Serialization_exception("unknown reference", ref_id);
    } else if (class_id == class_id_array) {
        read_array(obj);
    } else {
        read_object(obj, class_id);
    }
}

static Serializable_class* lookup_class(class_id_type class_id)
{
    Class_registry* registry = Base_module::module.instance->get_class_registry();
    assert(registry);
    return registry->lookup_class(class_id);
}

//
// class Abstract_serializer
//

void Abstract_serializer::write(long val)

{
    write(large(val));
}

void Abstract_serializer::write(unsigned long val)
{
    write(large(val));
}

void Abstract_serializer::write(large val)
{
    Large_value lval;
    lval.val = val;
    write(lval.a[0]);
    write(lval.a[1]);
}

void Abstract_serializer::write(ularge val)
{
    Large_value lval;
    lval.val = val;
    write(lval.a[0]);
    write(lval.a[1]);
}

void Abstract_serializer::write(float val)
{
    int* p = reinterpret_cast<int*>(&val);
    write(*p);
}

void Abstract_serializer::write(double val)
{
    large* p = reinterpret_cast<large*>(&val);
    write(*p);
}

void Abstract_serializer::write(quad val)
{
    Quad_value lval;
    lval.val = val;
    write(lval.a[0]);
    write(lval.a[1]);
    write(lval.a[2]);
    write(lval.a[3]);
}

void Abstract_serializer::write(const std::string& val)
{
    size_t slen = val.length();
    if (slen > (size_t) std::numeric_limits<int>::max())
        throw Serialization_exception("string length out of range");
    int ilen = (int) slen;
    write(ilen);
    if (ilen == 0)
        return;
    byte chk = (byte) LENGTH_VALIDATION_BYTE(ilen);
    write(chk);
    write((char*) val.c_str(), ilen);
}

void Abstract_serializer::write(const std::wstring& val)
{
    size_t slen = val.length();
    if (slen > (size_t) std::numeric_limits<int>::max())
        throw Serialization_exception("string length out of range");
    int ilen = (int) slen;
    write(ilen);
    if (ilen == 0)
        return;
    byte chk = (byte) LENGTH_VALIDATION_BYTE(ilen);
    write(chk);
    for (int i = 0; i < ilen; i++) {
        short c = val[i];
        write(c);
    }
}

//
// class Abstract_deserializer
//

void Abstract_deserializer::read(long& val)
{
    large lval;
    read(lval);
    val = (long) lval;
}

void Abstract_deserializer::read(unsigned long& val)
{
    large lval;
    read(lval);
    val = (unsigned long) lval;
}

void Abstract_deserializer::read(large& val)
{
    Large_value lval;
    read(lval.a[0]);
    read(lval.a[1]);
    val = lval.val;
}

void Abstract_deserializer::read(ularge& val)
{
    Large_value lval;
    read(lval.a[0]);
    read(lval.a[1]);
    val = lval.val;
}

void Abstract_deserializer::read(float& val)
{
    read(buffer, sizeof(float));
    long lval = ntohl(*((u_long*) buffer));
    val = *reinterpret_cast<float*>(&lval);
}

void Abstract_deserializer::read(double& val)
{
    large* p = reinterpret_cast<large*>(&val);
    read(*p);
}

void Abstract_deserializer::read(quad& val)
{
    Quad_value lval;
    read(lval.a[0]);
    read(lval.a[1]);
    read(lval.a[2]);
    read(lval.a[3]);
    val = lval.val;
}

void Abstract_deserializer::read(std::string& val)
{
    int slen;
    read(slen);
    if (slen == 0)
        return;
    byte chk;
    read(chk);
    if (slen < 0 || (byte) LENGTH_VALIDATION_BYTE(slen) != chk)
        throw Serialization_exception("invalid string length");
    val.resize(slen);
    read(&val[0], slen);
}

void Abstract_deserializer::read(std::wstring& val)
{
    int slen;
    read(slen);
    if (slen == 0)
        return;
    byte chk;
    read(chk);
    if (slen < 0 || (byte) LENGTH_VALIDATION_BYTE(slen) != chk)
        throw Serialization_exception("invalid string length");
    val.resize(slen);
    short tmp;
    for (int i = 0; i < slen; i++) {
        read(tmp);
        val[i] = tmp;
    }
}

void Abstract_deserializer::debug_print(std::ostream& out)
{
    const int n = 16;
    char buf[n];
    memset(buf, 0, n);
    large pos = tell();
    large off = n / 2;
    seek(pos - off);
    read(buf, n);
    seek(pos);
    for (int i = 0; i < n; i++) {
        int c = buf[i] & 0xff;
        char cc = 0x20 <= c && c <= 0x7f ? c : '?';
        bool mark = i == n / 2;
        if (mark)
            out << "-> 0x" << std::hex << c << " (" << cc << ") <- ";
        else
            out << "0x" << std::hex << c << " (" << cc << ") ";
    }
    out.flush();
}

//
// interface Serializable
//

std::string Serializable::class_id_to_string(class_id_type cid)
{
    char value[5];
    value[0] = (cid >> 24) & 0xff;
    value[1] = (cid >> 16) & 0xff;
    value[2] = (cid >>  8) & 0xff;
    value[3] = cid & 0xff;
    value[4] = 0;
    return value;
}

//
// class Serialization_exception
//

std::string Serialization_exception::get_message() const
{
    std::ostringstream stream;
    stream << Exception::get_message() << ": " << id << " (" << get_id_string_value() << ") cause: " << std::strerror(err);
    return stream.str();
}

std::string Serialization_exception::get_id_string_value() const
{
    return Serializable::class_id_to_string(id);
}

}}
