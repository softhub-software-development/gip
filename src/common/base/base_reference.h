
//
//  base_reference.h
//
//  Created by Softhub.
//  Copyright (c) 2019 Softhub. All rights reserved.
//

#ifndef BASE_REFERENCE_H
#define BASE_REFERENCE_H

#include "base_platform.h"
#include "base_types.h"
#include <string>

const class_id_type class_id_nil = 0;
const class_id_type class_id_array = -1;
const class_id_type class_id_ref = -2;

#define DECLARE_CLASS(id) \
    static const class_id_type class_id = id; \
    class_id_type get_class_id() const { return class_id; }

#define DECLARE_CLASS_VERSION(id, version) \
    static const class_id_type class_id = id; \
    static const version_type class_version = version; \
    class_id_type get_class_id() const { return class_id; }

#define FORWARD_CLASS(clazz) \
    class clazz; \
    typedef SOFTHUB::BASE::Reference<clazz> clazz##_ref; \
    typedef SOFTHUB::BASE::Reference<const clazz> clazz##_const_ref; \
    typedef SOFTHUB::BASE::Weak_reference<clazz> clazz##_weak_ref; \
    typedef SOFTHUB::BASE::Weak_reference<const clazz> clazz##_const_weak_ref;

namespace SOFTHUB {
namespace BASE {

class Interface;
class Serializable;
class Serializer;
class Deserializer;

template <typename T> class Reference;
template <typename T> class Weak_reference;

FORWARD_CLASS(Interface);
FORWARD_CLASS(Serializable);

typedef ularge object_id_type[4];

//
// interface Object_id
//

class Object_id {

    object_id_type value;

public:
    bool operator==(const Object_id& oid) const;
    size_t hash() const;
    void serialize(Serializer* serializer) const;
    void deserialize(Deserializer* deserializer);
};

//
// interface IAny
//

class IAny {
};

//
// interface IRetainable
//

class IRetainable : public IAny {

public:
    virtual int retain() const = 0;
    virtual int release() const = 0;
    virtual int release_internal() const = 0;
};

//
// Generic interface
//

class Interface : protected IRetainable {

    friend class Deserializer;

    template <typename T> friend class Reference;
    template <typename T> friend class Module;

public:
    virtual ~Interface() {}

    virtual Serializable* clone() const = 0;

    virtual class_id_type get_class_id() const = 0;
    virtual bool operator<(const Interface& obj) const = 0;
    virtual bool operator==(const Interface& obj) const = 0;
    virtual bool operator!=(const Interface& obj) const = 0;
    virtual bool equals(const Interface* obj) const = 0;
    virtual size_t hash() const = 0;

    static const class_id_type class_id = class_id_nil;
};

//
// interface Serializable
//

class Serializable : public virtual Interface {

public:
#ifdef UNIQUE_ID_SUPPORT
    virtual const Object_id& get_object_id() const = 0;
#endif
    virtual class_id_type get_class_id() const = 0;
    virtual void serialize(Serializer* serializer) const = 0;
    virtual void deserialize(Deserializer* deserializer) = 0;

    static std::string class_id_to_string(class_id_type cid);
    static const version_type class_version = 0;
};

//
// Object template class
//

template <typename I = Serializable>
class Object : public I {

    template <typename T> friend class Reference;
    template <typename T> friend class Module;
    friend class Deserializer;

    Object(const Object& obj);

    int release_internal() const;

protected:
    mutable int ref_count;
#ifdef UNIQUE_ID_SUPPORT
    mutable Object_id unique_id;

    const Object_id& get_object_id() const { return unique_id; }
#endif

    int retain() const;
    int release() const;

public:
    Object();
    virtual ~Object();

    virtual Serializable* copy() const;
    virtual Serializable* clone() const;

    int get_ref_count() const { return ref_count; }
    bool operator<(const Interface& obj) const { return this < &obj; }
    bool operator==(const Interface& obj) const { return this == &obj; }
    bool operator!=(const Interface& obj) const { return !this->operator==(obj); }
    bool equals(const Interface* obj) const { return this == obj; }
    size_t hash() const { return size_t(this); }

    // Inherited from Serializable
    class_id_type get_class_id() const { return class_id; }
    void serialize(Serializer* serializer) const {}
    void deserialize(Deserializer* deserializer) {}

    static const class_id_type class_id = class_id_nil;
};

//
// Template for objects derived from Object<Interface> or Interface
//

template <typename T = Object<> >
class Reference {

    template <typename C> friend class Reference;

    T* data;

public:
    Reference();
    Reference(T* data);
    Reference(const Reference& ref);
    template <typename C>
    explicit Reference(const Reference<C>& ref);
    ~Reference();

    const Reference& operator=(const Reference& ref);
    const Reference& operator=(T* obj);
    const T* operator->() const { assert(data); return data; }
    T* operator->() { assert(data); return data; }
    const T* get_pointer() const { return data; }
    T* get_pointer() { return data; }
    template <typename C> bool is_a() const { return dynamic_cast<C*>(data); }
    template <typename C> Reference<C> cast() const { return dynamic_cast<C*>(data); }
    bool operator!() const { return !data; }
    bool operator<(const Reference& ref) const { return data->operator<(*ref.data); }
    bool operator==(const Reference& ref) const;
    bool operator!=(const Reference& ref) const;
    operator bool() const { return data != 0; }
    operator bool() { return data != 0; }
    operator const T*() const { return data; }
    operator T*() { return data; }
//  operator const Reference<const T>&() const { return *this; }
//  operator Reference<T>&() { return *this; }

    static const class_id_type get_referenced_class_id() { return T::class_id; }
    typedef T value_type;
};

//
// class Weak_reference
//

template <typename T = Object<> >
class Weak_reference {

    T* data;

public:
    Weak_reference();
    Weak_reference(T* data);
    Weak_reference(const Reference<T>& ref);
    ~Weak_reference();

    const Weak_reference& operator=(const Reference<T>& ref);
    const Weak_reference& operator=(const Weak_reference& ref);
    const Weak_reference& operator=(T* obj);
    const T* get_pointer() const { return data; }
    T* get_pointer() { return data; }
    const T* operator->() const { assert(data); return data; }
    T* operator->() { assert(data); return data; }
    operator const T*() const { return data; }
    operator T*() { return data; }

    static const class_id_type get_referenced_class_id() { return T::class_id; }
};

}}

#ifdef UNIQUE_ID_SUPPORT
extern SOFTHUB::BASE::Object_id next_object_id();
#endif

#include "base_reference_inline.h"

#endif
