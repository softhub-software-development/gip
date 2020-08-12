
//
//  base_array.h
//
//  Created by Softhub.
//  Copyright (c) 2019 Softhub. All rights reserved.
//

#ifndef BASE_ARRAY_H
#define BASE_ARRAY_H

#include "base_reference.h"
#include "base_serialization.h"
#include "base_container.h"

namespace std {

template<typename T>
struct is_const<SOFTHUB::BASE::Reference<T> > : false_type {};

template<typename T>
struct is_const<SOFTHUB::BASE::Reference<const T> > : true_type {};

}

template<typename T, int sz> int sizeof_const_array(T(&)[sz]) { return sz; }

namespace SOFTHUB {
namespace BASE {

#define DECLARE_ARRAY(element_class, clazz) \
    typedef SOFTHUB::BASE::Array<element_class> clazz; \
    typedef SOFTHUB::BASE::Reference<clazz> clazz##_ref; \
    typedef SOFTHUB::BASE::Reference<const clazz> clazz##_const_ref;

//
// IArray Interface
//

class IArray : public IContainer {

public:
    virtual ~IArray() {}

    virtual class_id_type get_element_class_id() const = 0;
    virtual bool is_element_const() const = 0;
    virtual void reserve(size_t len) = 0;
    virtual void resize(size_t len) = 0;
};

//
// Array template class
//

template <typename T>
class Array : public Object<IArray> {

    friend class Deserializer;

    static bool default_comparator(const T& a, const T& b) { assert(a && b); return a < b; }

    std::vector<T>* elements;

    Array(const Array& array) : elements(array.elements) {}

public:
    Array();
    Array(size_t len);
    ~Array();

    Array<T>& operator=(const Array& array) { this->elements = array->elements; return *this; }
    const T& operator[](int index) const { return (*elements)[index]; }
    T& operator[](int index) { return (*elements)[index]; }
    const T& operator[](long index) const { return (*elements)[index]; }
    T& operator[](long index) { return (*elements)[index]; }
    const T& operator[](size_t index) const { return (*elements)[index]; }
    T& operator[](size_t index) { return (*elements)[index]; }
    class_id_type get_element_class_id() const { return T::get_referenced_class_id(); }
    bool is_element_const() const { return std::is_const<T>::value; }
    void reserve(size_t len) { elements->reserve(len); }
    void resize(size_t len) { elements->resize(len); }
    bool is_empty() const { return elements->empty(); }
    size_t get_size() const { return elements->size(); }
    size_t get_capacity() const { return elements->capacity(); }

    virtual void append(const T& element) { elements->push_back(element); }
    virtual void append(const Array* array);
    virtual void remove_all() { elements->clear(); }
    virtual bool equals(const Interface* obj) const;
    virtual void sort(bool descending = false);
    virtual bool binary_search(T& element, bool comp(const T&, const T&) = default_comparator) const;

    // iterator support
    typedef typename std::vector<T>::const_iterator const_iterator;
    typedef typename std::vector<T>::iterator iterator;
    const_iterator begin() const { return elements->begin(); }
    const_iterator end() const { return elements->end(); }
    iterator begin() { return elements->begin(); }
    iterator end() { return elements->end(); }

    // Inherited from Serializable
    class_id_type get_class_id() const { return class_id; }
    void serialize(Serializer* serializer) const;
    void deserialize(Deserializer* deserializer);

    static const class_id_type class_id = class_id_array;
};

//
// Template specialization for arrays
//

template <typename T>
class Reference<Array<T> > {

    template <typename C> friend class Reference;

    Array<T>* data;

public:
    Reference();
    Reference(size_t n);
    Reference(Array<T>* data);
    Reference(const Reference& ref);
    template <typename C>
    explicit Reference(const Reference<C>& ref);
    ~Reference();

    const Reference& operator=(const Reference& ref);
    const Array<T>* operator->() const { return data; }
    Array<T>* operator->() { return data; }
    const Array<T>* get_pointer() const { return data; }
    Array<T>* get_pointer() { return data; }
    template <typename C> bool is_a() const { return dynamic_cast<C*>(data); }
    template <typename C> Reference<Array<C> > cast() const { return dynamic_cast<Array<C>*>(data); }
    bool operator!() const { return !data; }
    const T& operator[](int index) const { return (*data)[index]; }
    T& operator[](int index) { return (*data)[index]; }
    const T& operator[](size_t index) const { return (*data)[index]; }
    T& operator[](size_t index) { return (*data)[index]; }
    bool operator==(const Reference<Array<T> >& ref) const;
    bool operator!=(const Reference<Array<T> >& ref) const;
    operator bool() const { return data != 0; }
    operator const Array<T>*() const { return data; }
    operator Array<T>*() { return data; }
};

//
// Template specialization for const arrays
//

template <typename T>
class Reference<const Array<T> > {

    template <typename C> friend class Reference;

    const Array<T>* data;

public:
    Reference();
    Reference(size_t n);
    Reference(const Array<T>* data);
    Reference(const Reference& ref);
    template <typename C>
    explicit Reference(const Reference<C>& ref);
    ~Reference();

    const Reference& operator=(const Reference& ref);
    const Array<T>* operator->() const { return data; }
    const Array<T>* get_pointer() const { return data; }
    bool operator!() const { return !data; }
    const T& operator[](int index) const { return (*data)[index]; }
    const T& operator[](long index) const { return (*data)[index]; }
    const T& operator[](size_t index) const { return (*data)[index]; }
    bool operator==(const Reference<const Array<T> >& ref) const;
    bool operator!=(const Reference<const Array<T> >& ref) const;
    operator bool() const { return data != 0; }
    operator const Array<T>*() const { return data; }
};

//
// Array_iterator class
//

template <typename T>
class Array_const_iterator {

    Reference<const IArray> array;
    mutable int index;

public:
    Array_const_iterator(const IArray* array, int index) : array(array), index(index) {}
    Array_const_iterator(const Array_const_iterator& it) : array(it.array), index(it.index) {}

    const T& operator*() const { assert(index >= 0); return Reference<const Array<T> >(array)[index-1]; }
    const Array_const_iterator& operator=(const Array_const_iterator& it);
    Array_const_iterator& operator++(int) { index++; return *this; }
    bool operator==(const Array_const_iterator& it) const { return array == it.array && index == it.index; }
    bool operator!=(const Array_const_iterator& it) const { return array != it.array || index != it.index; }
};

template <typename T>
class Array_iterator {

    Reference<IArray> array;
    mutable int index;

public:
    Array_iterator(IArray* array, int index) : array(array), index(index) {}
    Array_iterator(const Array_iterator& it) : array(it.array), index(it.index) {}

    const T& operator*() const { assert(index >= 0);  return Reference<const Array<T> >(array)[index-1]; }
    T& operator*() { assert(index >= 0); return Reference<Array<T> >(array)[index-1]; }
    const Array_iterator& operator=(const Array_iterator& it);
    Array_iterator& operator++(int) { index++; return *this; }
    bool operator==(const Array_iterator& it) const { return array == it.array && index == it.index; }
    bool operator!=(const Array_iterator& it) const { return array != it.array || index != it.index; }
};

}}

#include "base_array_inline.h"

#endif
