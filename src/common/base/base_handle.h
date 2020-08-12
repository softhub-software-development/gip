
//
//  handle.h
//
//  Created by Christian Lehner on 27/08/15.
//  Copyright (c) 2019 Softhub. All rights reserved.
//

#ifndef BASE_HANDLE_H
#define BASE_HANDLE_H

#include "base_reference.h"

namespace SOFTHUB {
namespace BASE {

//
// interface ILockable
//

class ILockable : virtual public Interface {

public:
    virtual bool is_locked() const = 0;
    virtual int lock() const = 0;
    virtual int unlock() const = 0;
};

//
// class Handle
//

template <typename T>
class Handle {

    template <typename C> friend class Handle;

    Reference<T> reference;

public:
    Handle() {}
    Handle(T* ptr);
    Handle(const Reference<T>& ref);
    Handle(const Handle<T>& handle);
    template <typename C>
    explicit Handle(const Handle<C>& ref);
    ~Handle();

    const Handle<T>& operator=(T* ptr);
    const Handle<T>& operator=(const Reference<T>& ref);
    const Handle<T>& operator=(const Handle<T>& handle);
    const T* operator->() const { return reference.operator->(); }
    T* operator->() { return reference.operator->(); }
    const Reference<T>& operator*() const { return reference; }
    Reference<T>& operator*() { return reference; }
    bool operator==(const Handle& handle) const;
    bool operator!=(const Handle& handle) const;
    bool operator<(const Handle& handle) const;
    bool operator!() { return !reference; }
    operator bool() const { return reference; }
    operator bool() { return reference; }
    operator T*() { return reference; }

    static const class_id_type get_referenced_class_id() { return T::class_id; }
    static const Handle<T> null;
};

}}

#include "base_handle_inline.h"

#endif
