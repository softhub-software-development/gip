
//
//  util_cache.h
//
//  Created by Christian Lehner on 4/25/15.
//  Copyright (c) 2019 Softhub. All rights reserved.
//

#ifndef LIB_UTIL_CACHE_H
#define LIB_UTIL_CACHE_H

#include <base/base.h>

namespace SOFTHUB {
namespace UTIL {

//
// class Persistent_cache
//

template <typename K, typename V, long SID = 0>
class Persistent_cache : public BASE::Cache<K,V,BASE::Object<> > {

    typedef BASE::Cache<K,V,BASE::Object<> > Cache_base;

public:
    Persistent_cache(int capacity = 10) : Cache_base(capacity) {}

    void serialize(BASE::Serializer* serializer) const;
    void deserialize(BASE::Deserializer* deserializer);

    DECLARE_CLASS(SID);
};

}}

#include "util_cache_inline.h"

#endif
