
/*
 *  util_choice_list.h
 *
 *  Created by Christian Lehner on 12/1/10.
 *  Copyright 2018 softhub. All rights reserved.
 */

#ifndef LIB_UTIL_CHOICE_LIST_H
#define LIB_UTIL_CHOICE_LIST_H

#include <base/base.h>

#define DECLARE_CHOICE_LIST(clazz, class_id) \
    FORWARD_CLASS(clazz##_list); \
    class clazz##_list : public UTIL::Choice_list<clazz> { \
    public: \
        clazz##_list() {} \
        DECLARE_CLASS(class_id); \
    };

namespace SOFTHUB {
namespace UTIL {

//
// Choice_list
//

template <typename T>
class Choice_list : public BASE::Object<> {

public:
    typedef T Choice_element;
    typedef BASE::Reference<T> Choice_element_ref;
    typedef BASE::Vector<Choice_element_ref> Choice_element_list;
    typedef BASE::Reference<Choice_element_list> Choice_element_list_ref;
    typedef typename Choice_element_list::iterator iterator;
    typedef typename Choice_element_list::const_iterator const_iterator;

private:
    Choice_element_list elements;
    Choice_element_ref current_element;

public:
    Choice_list() {}

    void add(Choice_element* element);
    void remove(Choice_element* element);
    void remove_all();
    void set_current_element(Choice_element* element);
    const Choice_element* get_current_element() const { return current_element; }
    Choice_element* get_current_element() { return current_element; }
    Choice_element_list& get_elements() { return elements; }
    size_t size() const { return elements.size(); }
    const Choice_element* operator[](int index) const { return elements[index]; }
    Choice_element* operator[](int index) { return elements[index]; }
    const_iterator begin() const { return elements.begin(); }
    const_iterator end() const { return elements.end(); }
    iterator begin() { return elements.begin(); }
    iterator end() { return elements.end(); }
    void serialize(BASE::Serializer* serializer) const;
    void deserialize(BASE::Deserializer* deserializer);
};

template <typename T>
void Choice_list<T>::add(Choice_element* element)
{
    elements.append(element);
    if (!current_element)
        current_element = element;
}

template <typename T>
void Choice_list<T>::remove(Choice_element* element)
{
    Choice_element_ref tmp(element);
    if (element == current_element)
        set_current_element(0);
    elements.remove(element);
}

template <typename T>
void Choice_list<T>::remove_all()
{
    set_current_element(0);
    elements.clear();
}

template <typename T>
void Choice_list<T>::set_current_element(Choice_element* element)
{
    assert(element ? elements.contains(element) : true);
    if (current_element)
        current_element->activate(false);
    current_element = element;
    if (element)
        element->activate(true);
}

template <typename T>
void Choice_list<T>::serialize(BASE::Serializer* serializer) const
{
    BASE::STL_util::serialize(serializer, elements);
    serializer->write(current_element);
}

template <typename T>
void Choice_list<T>::deserialize(BASE::Deserializer* deserializer)
{
    BASE::STL_util::deserialize(deserializer, elements);
    deserializer->read(current_element);
}

}}

#endif
