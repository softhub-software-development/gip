
//
//  base_reference.cpp
//
//  Created by Softhub.
//  Copyright (c) 2019 Softhub. All rights reserved.
//

#include "stdafx.h"
#include "base_serialization.h"

namespace SOFTHUB {
namespace BASE {

#ifdef UNIQUE_ID_SUPPORT

bool Object_id::operator==(const Object_id& oid) const
{
    for (int i = 0; i < 4; i++)
        if (value[i] != oid.value[i])
            return false;
    return true;
}

size_t Object_id::hash() const
{
    size_t val = value[0];
    for (int i = 1; i < 4; i++)
        val ^= value[i];
    return val;
}

void Object_id::serialize(Serializer* serializer) const
{
    for (int i = 0; i < 4; i++)
        serializer->write(value[i]);
}

void Object_id::deserialize(Deserializer* deserializer)
{
    for (int i = 0; i < 4; i++)
        deserializer->read(value[i]);
}

#endif

}}

#if defined UNIQUE_ID_SUPPORT && !defined NEXT_UNIQUE_ID_SUPPORT

SOFTHUB::BASE::Object_id next_object_id()
{
    assert(!"please provide implementation for next_object_id");
}

#endif
