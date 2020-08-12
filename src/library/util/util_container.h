
//
//  util_container.h
//
//  Created by Christian Lehner on 24/05/15.
//  Copyright (c) 2019 Softhub. All rights reserved.
//

#ifndef LIB_UTIL_CONTAINER_H
#define LIB_UTIL_CONTAINER_H

#include <base/base.h>
#include <net/net.h>

namespace SOFTHUB {
namespace UTIL {

//
// class Ordered_hash_map
//

template <typename K, typename V, long SID = 0, typename CON = BASE::Hash_map<K,V> >
class Ordered_hash_map : public BASE::Object<> {

public:
    typedef BASE::Vector<K> Keys;
    typedef typename CON::value_type value_type;
    typedef typename CON::const_reference const_reference;
    typedef typename CON::const_iterator const_iterator;
    typedef typename CON::iterator iterator;

private:
    static V null;

    Keys keys;
    CON container;
    size_t capacity;

public:
    Ordered_hash_map(size_t capacity = 10) : capacity(capacity) { keys.reserve(capacity); }

    size_t get_size() const { assert(keys.size() == container.size()); return container.size(); }
    size_t get_capacity() const { return capacity; }
    bool is_empty() const { assert(keys.empty()); return container.empty(); }
    const Keys& get_keys() const { return keys; }
    void push_back(const value_type& val) { store(val.first, val.second); }
    void store(const K& key, const V& val);
    void remove(const K& key);
    void remove(const Keys& keys_to_remove);
    void remove_all();
    bool contains(const K& key) const { return container.contains(key); }
    const V& find(const K& key) const;
    V& find(const K& key);
    void serialize(BASE::Serializer* serializer) const;
    void deserialize(BASE::Deserializer* deserializer);

    const_iterator begin() const { return container.begin(); }
    const_iterator end() const { return container.end(); }
    iterator begin() { return container.begin(); }
    iterator end() { return container.end(); }

    DECLARE_CLASS(SID);
};

//
// class Ordered_hash_map inline definitions
//

template <typename K, typename V, long SID, typename CON>
void Ordered_hash_map<K,V,SID,CON>::store(const K& key, const V& val)
{
#ifdef _DEBUG
    size_t n = container.size();
#endif
    const_iterator it = container.find(key);
    const_iterator tail = container.end();
    if (it == tail)
        keys.append(key);
    container.insert(key, val);
#ifdef _DEBUG
    if (it != tail) {
        assert(keys.size() == n);
        assert(container.size() == n);
    } else {
        assert(keys.size() == n+1);
        assert(container.size() == n+1);
    }
#endif
}

template <typename K, typename V, long SID, typename CON>
void Ordered_hash_map<K,V,SID,CON>::remove(const K& key)
{
    const_iterator it = container.find(key);
    const_iterator tail = container.end();
    if (it != tail) {
        keys.remove(key);
        container.remove(key);
    }
}

template <typename K, typename V, long SID, typename CON>
void Ordered_hash_map<K,V,SID,CON>::remove(const Keys& keys_to_remove)
{
    typename Keys::const_iterator it = keys_to_remove.begin();
    typename Keys::const_iterator tail = keys_to_remove.end();
    while (it != tail) {
        const K& key = *it++;
        keys.remove(key);
        container.remove(key);
    }
}

template <typename K, typename V, long SID, typename CON>
void Ordered_hash_map<K,V,SID,CON>::remove_all()
{
    keys.clear();
    container.clear();
    assert(keys.size() == 0);
    assert(container.size() == 0);
}

template <typename K, typename V, long SID, typename CON>
const V& Ordered_hash_map<K,V,SID,CON>::find(const K& key) const
{
    typename BASE::Hash_map<K,V>::const_iterator it = container.find(key);
    typename BASE::Hash_map<K,V>::const_iterator tail = container.end();
    return it == tail ? null : it->second;
}

template <typename K, typename V, long SID, typename CON>
V& Ordered_hash_map<K,V,SID,CON>::find(const K& key)
{
    typename BASE::Hash_map<K,V>::iterator it = container.find(key);
    typename BASE::Hash_map<K,V>::iterator tail = container.end();
    return it == tail ? null : it->second;
}

template <typename K, typename V, long SID, typename CON>
void Ordered_hash_map<K,V,SID,CON>::serialize(BASE::Serializer* serializer) const
{
    int n = (int) keys.size(), len = (int) capacity;
    serializer->write(n);
    serializer->write(len);
    typename Keys::const_iterator it = keys.begin();
    typename Keys::const_iterator tail = keys.end();
    while (it != tail) {
        const K& key = *it++;
        const V& val = find(key);
        serializer->write(key);
        serializer->write(val);
    }
}

template <typename K, typename V, long SID, typename CON>
void Ordered_hash_map<K,V,SID,CON>::deserialize(BASE::Deserializer* deserializer)
{
    int n, len;
    deserializer->read(n);
    deserializer->read(len);
    keys.reserve(len);
    container.reserve(n);
    capacity = len;
    for (int i = 0; i < n; i++) {
        K key; V val;
        deserializer->read(key);
        deserializer->read(val);
        keys.append(key);
        container.insert(key, val);
    }
#ifdef _DEBUG
    size_t k = keys.size();
    size_t c = container.size();
    assert(n == k && k == c);
#endif
}

template <typename K, typename V, long SID, typename CON>
V Ordered_hash_map<K,V,SID,CON>::null;

}}

#endif
