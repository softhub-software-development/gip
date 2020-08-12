
//
//  base_stl_util.h
//
//  Created by Softhub.
//  Copyright (c) 2019 Softhub. All rights reserved.
//

#ifndef BASE_STL_UTIL_H
#define BASE_STL_UTIL_H

#include "base_serialization.h"

namespace SOFTHUB {
namespace BASE {

template <typename T> class Array;

//
// class STL_util
//

class STL_util {

public:
    template <typename T> static void copy(const Hash_set<T>& set, Reference<Array<T> >& array);
    template <typename T> static void copy(const Reference<Array<T> >& array, Hash_set<T>& set);
    template <typename T> static void copy(const Array<T>* array, Hash_set<T>& set);
    template <typename T> static size_t hash(const List<T>& list);
    template <typename T> static size_t hash(const Vector<T>& list);
    template <typename T> static size_t hash(const Hash_set<T>& set);

    template <typename T, typename S = Serializer>
    static void serialize(S* serializer, const List<T>& list);
    template <typename T, typename D = Deserializer>
    static void deserialize(D* deserializer, List<T>& list);
    template <typename T, typename S = Serializer>
    static void serialize(S* serializer, const Vector<T>& vec);
    template <typename T, typename D = Deserializer>
    static void deserialize(D* deserializer, Vector<T>& vec);
    template <typename T, typename S = Serializer>
    static void serialize(S* serializer, const Hash_set<T>& set);
    template <typename T, typename D = Deserializer>
    static void deserialize(D* deserializer, Hash_set<T>& set);
    template <typename K, typename V, typename S = Serializer>
    static void serialize(S* serializer, const Hash_map<K,V>& map);
    template <typename K, typename V, typename D = Deserializer>
    static void deserialize(D* deserializer, Hash_map<K,V>& map);
    template <typename K, typename V, typename S = Serializer>
    static void serialize(S* serializer, const Hash_array<K,V>& assoc);
    template <typename K, typename V, typename D = Deserializer>
    static void deserialize(D* deserializer, Hash_array<K,V>& assoc);

    template <typename T> static std::string class_name_of(T* obj);

    static std::string concat(const String_vector& list);
};

}}

#include "base_stl_util_inline.h"

#endif
