
//
//  util_cache_inline.h
//
//  Created by Christian Lehner on 4/25/15.
//  Copyright (c) 2019 Softhub. All rights reserved.
//

#ifndef LIB_UTIL_CACHE_INLINE_H
#define LIB_UTIL_CACHE_INLINE_H

#include "util_cache.h"
#include <iostream>

namespace SOFTHUB {
namespace UTIL {

//
// class Cache
//

template <typename K, typename V, long SID>
void Persistent_cache<K,V,SID>::serialize(BASE::Serializer* serializer) const
{
    int cap = (int) Cache_base::get_capacity();
    int n = (int) Cache_base::get_size();
    serializer->write(cap);
    serializer->write(n);
    typename BASE::List<K>::const_iterator it = Cache_base::list.begin();
    typename BASE::List<K>::const_iterator tail = Cache_base::list.end();
    while (it != tail) {
        const K& key = *it++;
        serializer->write(key);
        const V& val = Cache_base::container.get(key);
        serializer->write(val);
    }
}

template <typename K, typename V, long SID>
void Persistent_cache<K,V,SID>::deserialize(BASE::Deserializer* deserializer)
{
    int cap, n;
    deserializer->read(cap);
    Cache_base::set_capacity(cap);
    deserializer->read(n);
    Cache_base::container.reserve(n);
    for (size_t i = 0; i < n; i++) {
        K key; V val;
        deserializer->read(key);
        deserializer->read(val);
        Cache_base::list.append(key);
        Cache_base::container.insert(key, val);
    }
}

}}

#endif
