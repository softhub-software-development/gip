
//
//  base_handle_inline.h
//
//  Created by Christian Lehner on 06/05/17.
//  Copyright (c) 2019 Softhub. All rights reserved.
//

#ifndef BASE_HANDLE_INLINE_H
#define BASE_HANDLE_INLINE_H

namespace SOFTHUB {
namespace BASE {

//
// class Handle
//

template <typename T>
const Handle<T> Handle<T>::null;

template <typename T>
Handle<T>::Handle(T* ptr) : reference(ptr)
{
    if (reference)
        reference->lock();
}

template <typename T>
Handle<T>::Handle(const Reference<T>& ref) : reference(ref)
{
    if (reference)
        reference->lock();
}

template <typename T>
Handle<T>::Handle(const Handle<T>& handle) : reference(handle.reference)
{
    if (reference)
        reference->lock();
}

template <typename T>
template <typename C>
Handle<T>::Handle(const Handle<C>& handle) : reference(handle.reference)
{
    if (reference)
        reference->lock();
}

template <typename T>
Handle<T>::~Handle()
{
    if (reference)
        reference->unlock();
}

template <typename T>
const Handle<T>& Handle<T>::operator=(T* ptr)
{
    if (ptr)
        ptr->lock();
    if (reference)
        reference->unlock();
    reference = ptr;
    assert(reference ? reference->is_locked() : true);
    return *this;
}

template <typename T>
const Handle<T>& Handle<T>::operator=(const Reference<T>& ref)
{
    if (ref)
        ref->lock();
    if (reference)
        reference->unlock();
    reference = ref;
    assert(reference ? reference->is_locked() : true);
    return *this;
}

template <typename T>
const Handle<T>& Handle<T>::operator=(const Handle<T>& handle)
{
    if (handle.reference)
        handle.reference->lock();
    if (reference)
        reference->unlock();
    reference = handle.reference;
    assert(reference ? reference->is_locked() : true);
    return *this;
}

template <typename T>
bool Handle<T>::operator==(const Handle<T>& handle) const
{
    if (reference == handle.reference)
        return true;
    if (!reference || !handle.reference)
        return false;
    return *reference == *handle.reference;
}

template <typename T>
bool Handle<T>::operator!=(const Handle<T>& handle) const
{
    return !operator==(handle);
}

template <typename T>
bool Handle<T>::operator<(const Handle& handle) const
{
    return reference->operator<(*handle.reference);
}

}}

#endif
