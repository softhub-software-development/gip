
//
//  base_class_registry.h
//
//  Created by Softhub.
//  Copyright (c) 2019 Softhub. All rights reserved.
//

#ifndef BASE_CLASS_REGISTRY_H
#define BASE_CLASS_REGISTRY_H

#include "base_reference.h"
#include "base_container.h"
#include "base_array.h"
#include "base_dictionary.h"

namespace SOFTHUB {
namespace BASE {

//
// interface Serializable_class
//

class Serializable_class {

public:
    virtual ~Serializable_class() {}

    virtual class_id_type get_class_id() const = 0;
    virtual version_type get_version() const = 0;
    virtual Serializable* create() const = 0;
    virtual Serializable* create_array() const = 0;
    virtual Serializable* create_array_const() const = 0;
};

//
// class Abstract_class
//

template <typename T>
class Abstract_class : public Serializable_class {

    class_id_type class_id;

public:
    Abstract_class() : class_id(T::class_id) {}

    class_id_type get_class_id() const { return class_id; }
    version_type get_version() const { return T::class_version; }
    Serializable* create() const { throw new Exception("cannot create abstract class"); }
    Serializable* create_array() const { return new Array<Reference<T> >(); }
    Serializable* create_array_const() const { return new Array<Reference<const T> >(); }
};

//
// class Class
//

template <typename T>
class Class : public Abstract_class<T> {

public:
    Class() {}

    Serializable* create() const { return new T(); }
};

//
// class Class_info
//

class Class_info : public Serializable_class {

    class_id_type class_id;
    version_type version;

public:
    Class_info(class_id_type class_id, version_type version) : class_id(class_id), version(version) {}

    class_id_type get_class_id() const { return class_id; }
    version_type get_version() const { return version; }
    Serializable* create() const { assert(!"abstract class info"); return 0; }
    Serializable* create_array() const { assert(!"abstract class info"); return 0; }
    Serializable* create_array_const() const { assert(!"abstract class info"); return 0; }
};

//
// class Class_registry
//

class Class_registry {

    typedef Hash_map<class_id_type,Serializable_class*> Id_class_map;

    int id_counter;
    Id_class_map id_class_map;

public:
    Class_registry();
    ~Class_registry();

    void register_class(Serializable_class* clazz);
    void unregister_class(Serializable_class* clazz);
    void unregister_all_classes();
    Serializable_class* lookup_class(class_id_type class_id) const;
    void serialize(Serializer* serializer) const;
    void deserialize(Deserializer* deserializer);
};

//
// Class_registry_exception
//

class Class_registry_exception : public Exception {

    class_id_type class_id;

public:
    Class_registry_exception(const std::string& msg, class_id_type class_id);

    std::string get_message() const;
};

}}

#endif
