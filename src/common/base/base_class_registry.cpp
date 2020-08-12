
//
//  base_class_registry.cpp
//
//  Created by Softhub.
//  Copyright (c) 2019 Softhub. All rights reserved.
//

#include "stdafx.h"
#include "base_class_registry.h"
#include "base_serialization.h"
#include <sstream>

namespace SOFTHUB {
namespace BASE {

using namespace std;

//
// Class_registry
//

Class_registry::Class_registry()
{
}

Class_registry::~Class_registry()
{
    Id_class_map::const_iterator iterator = id_class_map.begin();
    Id_class_map::const_iterator end = id_class_map.end();
    while (iterator != end) {
        Serializable_class* clazz = iterator->second;
        delete clazz;
        iterator++;
    }
}

void Class_registry::register_class(Serializable_class* clazz)
{
    class_id_type class_id = clazz->get_class_id();
    Id_class_map::const_iterator iterator = id_class_map.find(class_id);
    Id_class_map::const_iterator end = id_class_map.end();
    if (iterator != end)
        throw Class_registry_exception("class already registered", class_id);
    id_class_map[class_id] = clazz;
}

void Class_registry::unregister_class(Serializable_class* clazz)
{
    class_id_type class_id = clazz->get_class_id();
    if (!id_class_map.remove(class_id))
        throw Class_registry_exception("class not registered", class_id);
}

void Class_registry::unregister_all_classes()
{
    id_class_map.clear();
}

Serializable_class* Class_registry::lookup_class(class_id_type class_id) const
{
    Id_class_map::const_iterator iterator = id_class_map.find(class_id);
    if (iterator == id_class_map.end())
        return 0;
    Serializable_class* clazz = iterator->second;
    assert(clazz != 0);
    return clazz;
}

void Class_registry::serialize(Serializer* serializer) const
{
    int num_classes = (int) id_class_map.size();
    serializer->write(id_counter);
    serializer->write(num_classes);
    Id_class_map::const_iterator it = id_class_map.begin();
    Id_class_map::const_iterator tail = id_class_map.end();
    while (it != tail) {
        Id_class_map::value_type val = *it++;
        class_id_type uid = val.first;
        class_id_type cid = val.second->get_class_id();
        version_type version = val.second->get_version();
        serializer->write(uid);
        serializer->write(cid);
        serializer->write(version);
    }
}

void Class_registry::deserialize(Deserializer* deserializer)
{
    int num_classes;
    deserializer->read(id_counter);
    deserializer->read(num_classes);
    for (int i = 0; i < num_classes; i++) {
        class_id_type uid, cid;
        version_type version;
        deserializer->read(uid);
        deserializer->read(cid);
        deserializer->read(version);
        id_class_map.insert(uid, new Class_info(cid, version));
    }
}

//
// Class_registry_exception
//

Class_registry_exception::Class_registry_exception(const string& msg, class_id_type class_id) :
    Exception(msg), class_id(class_id)
{
}

string Class_registry_exception::get_message() const
{
    ostringstream stream;
    stream << what() << ": " << class_id << " (" << Serializable::class_id_to_string(class_id) << ")";
    return stream.str();
}

}}
