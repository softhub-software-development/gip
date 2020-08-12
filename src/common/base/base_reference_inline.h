
//
//  base_reference_inline.h
//
//  Created by Softhub.
//  Copyright (c) 2019 Softhub. All rights reserved.
//

#ifndef BASE_REFERENCE_INLINE_H
#define BASE_REFERENCE_INLINE_H

#include "base_exception.h"

namespace SOFTHUB {
namespace BASE {

//
// class Reference
//

template <typename T>
Reference<T>::Reference() : data(0)
{
}

template <typename T>
Reference<T>::Reference(T* data) : data(data)
{
    if (data)
        data->retain();
}

template <typename T>
Reference<T>::Reference(const Reference& ref) : data(ref.data)
{
    if (data)
        data->retain();
}

template <typename T>
template <typename C>
Reference<T>::Reference(const Reference<C>& ref) : data(dynamic_cast<T*>(ref.data))
{
    assert(ref.data ? data != 0 : true);
    if (data)
        data->retain();
}

template <typename T>
Reference<T>::~Reference()
{
    if (data)
        data->release();
}

template <typename T>
const Reference<T>& Reference<T>::operator=(const Reference<T>& ref)
{
    if (ref.data)
        ref.data->retain();
    if (data)
        data->release();
    data = ref.data;
    return *this;
}

template <typename T>
const Reference<T>& Reference<T>::operator=(T* obj)
{
    if (obj)
        obj->retain();
    if (data)
        data->release();
    data = obj;
    return *this;
}

template <typename T>
bool Reference<T>::operator==(const Reference<T>& ref) const
{
    if (data == ref.data)
        return true;
    if (data == 0 || ref.data == 0)
        return false;
    return *data == *ref.data;
}

template <typename T>
bool Reference<T>::operator!=(const Reference<T>& ref) const
{
    return !operator==(ref);
}

//
// class Weak_reference
//

template <typename T>
Weak_reference<T>::Weak_reference() : data(0)
{
}

template <typename T>
Weak_reference<T>::Weak_reference(T* data) : data(data)
{
}

template <typename T>
Weak_reference<T>::Weak_reference(const Reference<T>& ref) : data(ref.data)
{
}

template <typename T>
Weak_reference<T>::~Weak_reference()
{
}

template <typename T>
const Weak_reference<T>& Weak_reference<T>::operator=(const Reference<T>& ref)
{
    data = ref.data;
    return *this;
}

template <typename T>
const Weak_reference<T>& Weak_reference<T>::operator=(const Weak_reference& ref)
{
    data = ref.data;
    return *this;
}

template <typename T>
const Weak_reference<T>& Weak_reference<T>::operator=(T* obj)
{
    data = obj;
    return *this;
}

//
// class Object
//

template <typename T>
Object<T>::Object() : ref_count(0)
{
#ifdef UNIQUE_ID_SUPPORT
    unique_id = ::next_object_id();
#endif
}

template <typename T>
Object<T>::~Object()
{
}

template <typename T>
int Object<T>::retain() const
{
    return ++ref_count;
}

template <typename T>
int Object<T>::release() const
{
    assert(ref_count > 0);
    if (--ref_count > 0)
        return ref_count;
    delete this;
    return 0;
}

template <typename T>
int Object<T>::release_internal() const
{
    assert(ref_count > 0);
    return --ref_count;
}

template <typename T>
Serializable* Object<T>::copy() const
{
    throw Exception("shallow copy not supported");
}

template <typename T>
Serializable* Object<T>::clone() const
{
    throw Exception("deep clone not supported");
}

}}

#endif
