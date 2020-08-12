
//
//  base_array_inline.h
//
//  Created by Softhub.
//  Copyright (c) 2019 Softhub. All rights reserved.
//

#ifndef BASE_ARRAY_INLINE_H
#define BASE_ARRAY_INLINE_H

#include <algorithm>
#include <vector>

namespace SOFTHUB {
namespace BASE {

//
// Array<T>
//

template <typename T>
Array<T>::Array() : elements(new std::vector<T>())
{
}

template <typename T>
Array<T>::Array(size_t len) : elements(new std::vector<T>(len))
{
}

template <typename T>
Array<T>::~Array()
{
    assert(ref_count >= 0);
    if (ref_count == 0)
        delete elements;
}

template <typename T>
void Array<T>::append(const Array* array)
{
    for (size_t i = 0, n = array->get_size(); i < n; i++)
        append((*array)[i]);
}

template <typename T>
bool Array<T>::equals(const Interface* obj) const
{
    const Array<T>* a = dynamic_cast<const Array<T>*>(obj);
    if (a == 0)
        return false;
    size_t n = get_size();
    if (n != a->get_size())
        return false;
    for (size_t i = 0; i < n; i++) {
        const T& e = (*this)[i];
        const T& f = (*a)[i];
        if (e != f)
            return false;
    }
    return true;
}

template <typename T>
void Array<T>::sort(bool descending)
{
    if (descending)
        std::sort(elements->rbegin(), elements->rend());
    else
        std::sort(elements->begin(), elements->end());
}

template <typename T>
bool Array<T>::binary_search(T& element, bool comp(const T&, const T&)) const
{
    const_iterator head = elements->begin();
    const_iterator tail = elements->end();
    const_iterator null;
    if (head == tail)
        return false;
    const_iterator it = std::lower_bound(head, tail, element, comp);
    bool found = it != tail;
    if (found)
        element = *it;
    return found;
}

template <typename T>
void Array<T>::serialize(Serializer* serializer) const
{
    large count = get_size();
    large capacity = get_capacity();
    assert(count >= 0 && capacity >= 0);
    serializer->write(count);
    serializer->write(capacity);
    for (size_t i = 0; i < (size_t) count; i++) {
        const T& obj = (*elements)[i];
        serializer->write(obj);
    }
}

template <typename T>
void Array<T>::deserialize(Deserializer* deserializer)
{
    large count = 0, capacity = 0;
    deserializer->read(count);
    deserializer->read(capacity);
    // tell us if we allocate huge junks of memory
    assert(0 <= capacity && capacity < (1 << 23));
    elements->reserve((size_t) capacity);
    for (size_t i = 0; i < (size_t) count; i++) {
        T obj;
        deserializer->read(obj);
        append(obj);
    }
}

//
// Reference template specialization for Array<T>
//

template <typename T>
Reference<Array<T> >::Reference() : data(0)
{
}

template <typename T>
Reference<Array<T> >::Reference(size_t n)
{
    data = new Array<T>(n);
    data->retain();
}

template <typename T>
Reference<Array<T> >::Reference(Array<T>* data) : data(data)
{
    if (data)
        data->retain();
}

template <typename T>
Reference<Array<T> >::Reference(const Reference& ref) : data(ref.data)
{
    if (data)
        data->retain();
}

template <typename T>
template <typename C>
Reference<Array<T> >::Reference(const Reference<C>& ref) : data(dynamic_cast<Array<T> *>(ref.data))
{
    assert(ref.data ? data != 0 : true);
    if (data)
        data->retain();
}

template <typename T>
Reference<Array<T> >::~Reference()
{
    if (data)
        data->release();
}

template <typename T>
const Reference<Array<T> >& Reference<Array<T> >::operator=(const Reference<Array<T> >& ref)
{
    if (ref.data)
        ref.data->retain();
    if (data)
        data->release();
    data = ref.data;
    return *this;
}

template <typename T>
bool Reference<Array<T> >::operator==(const Reference<Array<T> >& ref) const
{
    if (data == ref.data)
        return true;
    if (data == 0 || ref.data == 0)
        return false;
    return *data == *ref.data;
}

template <typename T>
bool Reference<Array<T> >::operator!=(const Reference<Array<T> >& ref) const
{
    return !operator==(ref);
}

//
// Reference template specialization for const Array<T>
//

template <typename T>
Reference<const Array<T> >::Reference() : data(0)
{
}

template <typename T>
Reference<const Array<T> >::Reference(size_t n)
{
    data = new Array<T>(n);
    data->retain();
}

template <typename T>
Reference<const Array<T> >::Reference(const Array<T>* data) : data(data)
{
    if (data)
        data->retain();
}

template <typename T>
Reference<const Array<T> >::Reference(const Reference& ref) : data(ref.data)
{
    if (data)
        data->retain();
}

template <typename T>
template <typename C>
Reference<const Array<T> >::Reference(const Reference<C>& ref) : data(dynamic_cast<const Array<T> *>(ref.data))
{
    assert(ref.data ? data != 0 : true);
    if (data)
        data->retain();
}

template <typename T>
Reference<const Array<T> >::~Reference()
{
    if (data)
        data->release();
}

template <typename T>
const Reference<const Array<T> >& Reference<const Array<T> >::operator=(const Reference<const Array<T> >& ref)
{
    if (ref.data)
        ref.data->retain();
    if (data)
        data->release();
    data = ref.data;
    return *this;
}

template <typename T>
bool Reference<const Array<T> >::operator==(const Reference<const Array<T> >& ref) const
{
    if (data == ref.data)
        return true;
    if (data == 0 || ref.data == 0)
        return false;
    return *data == *ref.data;
}

template <typename T>
bool Reference<const Array<T> >::operator!=(const Reference<const Array<T> >& ref) const
{
    return !operator==(ref);
}

}}

#endif
