
//
//  base_serialization_inline.h
//
//  Created by Softhub.
//  Copyright (c) 2019 Softhub. All rights reserved.
//

#ifndef BASE_SERIALIZATION_INLINE_H
#define BASE_SERIALIZATION_INLINE_H

namespace SOFTHUB {
namespace BASE {

//
// class Serializer
//

template <typename T>
void Serializer::write(const Reference<T>& ref)
{
    const Serializable* obj = dynamic_cast<const Serializable*>(ref.get_pointer());
    if ((ref.get_pointer() != 0) != (obj != 0))
        throw Serialization_exception("serializing invalid class", T::class_id);
    write(obj, T::class_id);
}

template <typename T>
void Serializer::write(const Weak_reference<T>& ref)
{
    const Serializable* obj = dynamic_cast<const Serializable*>(ref.get_pointer());
    if ((ref.get_pointer() != 0) != (obj != 0))
        throw Serialization_exception("serializing invalid class", T::class_id);
    write(obj, T::class_id);
}

template <typename T>
void Serializer::write(const Handle<T>& handle)
{
    Reference<T> ref = *handle;
    write(ref, T::class_id);
}

//
// class Deserializer
//

template <typename T>
void Deserializer::read(Reference<T>& ref)
{
    Serializable* obj = const_cast<Serializable*>(dynamic_cast<const Serializable*>(ref.get_pointer()));
    read(obj, T::class_id);
    T* t = dynamic_cast<T*>(obj);
    if ((t != 0) != (obj != 0))
        throw Serialization_exception("deserializing invalid class", T::class_id);
    ref = Reference<T>(t);
}

template <typename T>
void Deserializer::read(Weak_reference<T>& ref)
{
    Serializable* obj = const_cast<Serializable*>(dynamic_cast<const Serializable*>(ref.get_pointer()));
    read(obj, T::class_id);
    T* t = dynamic_cast<T*>(obj);
    if ((t != 0) != (obj != 0))
        throw Serialization_exception("deserializing invalid class", T::class_id);
    ref = Weak_reference<T>(t);
}

template <typename T>
void Deserializer::read(Handle<T>& handle)
{
    Reference<T> ref;
    read(ref);
    handle = Handle<T>(ref);
}

}}

#endif
