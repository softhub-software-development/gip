
//
//  base_stl_util_inline.h
//
//  Created by Softhub.
//  Copyright (c) 2019 Softhub. All rights reserved.
//

#ifndef BASE_STL_UTIL_INLINE_H
#define BASE_STL_UTIL_INLINE_H

#ifdef _DEBUG
#include "base_stl_util.h"
#endif

#include <typeinfo>
#ifndef PLATFORM_WIN
#include <cxxabi.h>
#endif

namespace SOFTHUB {
namespace BASE {

//
// class STL_util
//

template <typename T>
void STL_util::copy(const Hash_set<T>& set, Reference<Array<T> >& array)
{
    typename Hash_set<T>::const_iterator it = set.begin();
    typename Hash_set<T>::const_iterator tail = set.end();
    while (it != tail) {
        const T& obj = *it++;
        array->append(obj);
    }
}

template <typename T>
void STL_util::copy(const Reference<Array<T> >& array, Hash_set<T>& set)
{
    typename Array<T>::const_iterator it = array->begin();
    typename Array<T>::const_iterator tail = array->end();
    while (it != tail) {
        const T& obj = *it++;
        set.insert(obj);
    }
}

template <typename T>
void STL_util::copy(const Array<T>* array, Hash_set<T>& set)
{
    typename Array<T>::const_iterator it = array->begin();
    typename Array<T>::const_iterator tail = array->end();
    while (it != tail) {
        const T& obj = *it++;
        set.insert(obj);
    }
}

template <typename T>
size_t STL_util::hash(const List<T>& list)
{
    size_t value = 0;
    typename List<T>::const_iterator it = list.begin();
    typename List<T>::const_iterator tail = list.end();
    while (it != tail) {
        const T& obj = *it++;
        value += std::hash<T>()(obj);
    }
    return value;
}

template <typename T>
size_t STL_util::hash(const Vector<T>& list)
{
    size_t value = 0;
    typename Vector<T>::const_iterator it = list.begin();
    typename Vector<T>::const_iterator tail = list.end();
    while (it != tail) {
        const T& obj = *it++;
        value += std::hash<T>()(obj);
    }
    return value;
}

template <typename T>
size_t STL_util::hash(const Hash_set<T>& set)
{
    size_t value = 0;
    typename Hash_set<T>::const_iterator it = set.begin();
    typename Hash_set<T>::const_iterator tail = set.end();
    while (it != tail) {
        const T& obj = *it++;
        value += std::hash<T>()(obj);
    }
    return value;
}

template <typename T, typename S>
void STL_util::serialize(S* serializer, const List<T>& list)
{
    serializer->write((unsigned) list.size());
    typename List<T>::const_iterator it = list.begin();
    typename List<T>::const_iterator tail = list.end();
    while (it != tail) {
        const T& obj = *it++;
        serializer->write(obj);
    }
}

template <typename T, typename D>
void STL_util::deserialize(D* deserializer, List<T>& list)
{
    unsigned n;
    deserializer->read(n);
    for (unsigned i = 0; i < n; i++) {
        T obj;
        deserializer->read(obj);
        list.append(obj);
    }
}

template <typename T, typename S>
void STL_util::serialize(S* serializer, const Vector<T>& vec)
{
    serializer->write((unsigned) vec.size());
    typename Vector<T>::const_iterator it = vec.begin();
    typename Vector<T>::const_iterator tail = vec.end();
    while (it != tail) {
        const T& obj = *it++;
        serializer->write(obj);
    }
}

template <typename T, typename D>
void STL_util::deserialize(D* deserializer, Vector<T>& vec)
{
    unsigned n;
    deserializer->read(n);
    for (unsigned i = 0; i < n; i++) {
        T obj;
        deserializer->read(obj);
        vec.append(obj);
    }
}

template <typename T, typename S>
void STL_util::serialize(S* serializer, const Hash_set<T>& set)
{
    serializer->write((unsigned) set.size());
    typename Hash_set<T>::const_iterator it = set.begin();
    typename Hash_set<T>::const_iterator tail = set.end();
    while (it != tail) {
        const T& obj = *it++;
        serializer->write(obj);
    }
}

template <typename T, typename D>
void STL_util::deserialize(D* deserializer, Hash_set<T>& set)
{
    unsigned n;
    deserializer->read(n);
    for (unsigned i = 0; i < n; i++) {
        T obj;
        deserializer->read(obj);
        set.insert(obj);
    }
}

template <typename K, typename V, typename S>
void STL_util::serialize(S* serializer, const Hash_map<K,V>& map)
{
    serializer->write((unsigned) map.size());
    typename Hash_map<K,V>::const_iterator it = map.begin();
    typename Hash_map<K,V>::const_iterator tail = map.end();
    while (it != tail) {
        const typename Hash_map<K,V>::value_type& pair = *it++;
        serializer->write(pair.first);
        serializer->write(pair.second);
    }
}

template <typename K, typename V, typename D>
void STL_util::deserialize(D* deserializer, Hash_map<K,V>& map)
{
    unsigned n;
    deserializer->read(n);
    for (unsigned i = 0; i < n; i++) {
        K key; V val;
        deserializer->read(key);
        deserializer->read(val);
        map.insert(key, val);
    }
}

template <typename K, typename V, typename S>
void STL_util::serialize(S* serializer, const Hash_array<K,V>& assoc)
{
    unsigned n = (unsigned) assoc.size();
    serializer->write(n);
    for (unsigned i = 0; i < n; i++) {
        const K& key = assoc.key_at(i);
        const V& val = assoc.get(key);
        serializer->write(key);
        serializer->write(val);
    }
}

template <typename K, typename V, typename D>
void STL_util::deserialize(D* deserializer, Hash_array<K,V>& assoc)
{
    unsigned n;
    deserializer->read(n);
    assoc.clear();
    for (unsigned i = 0; i < n; i++) {
        K key; V val;
        deserializer->read(key);
        deserializer->read(val);
#ifndef NDEBUG
        bool inserted =
#endif
        assoc.insert(key, val);
        assert(inserted);
    }
}

template <typename T>
std::string STL_util::class_name_of(T* obj)
{
#ifdef PLATFORM_WIN
	return "<unknown>";
#else
	int status;
	const char* internal_name = typeid(*obj).name();
	return abi::__cxa_demangle(internal_name, 0, 0, &status);
#endif
}

inline std::string STL_util::concat(const String_vector& list)
{
    std::string str;
    String_vector::const_iterator it = list.begin();
    String_vector::const_iterator tail = list.end();
    while (it != tail) {
        str += *it++;
    }
    return str;
}

}}

#endif
