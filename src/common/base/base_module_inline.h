
//
//  base_module_inline.h
//
//  Created by Christian Lehner on 6/7/12.
//  Copyright (c) 2019 Softhub. All rights reserved.
//

#ifndef BASE_MODULE_INLINE_H
#define BASE_MODULE_INLINE_H

#ifdef _DEBUG
#include "base_stl_util.h"
#endif
#include <type_traits>

namespace SOFTHUB {
namespace BASE {

//
// class Module
//

template <typename T> T* Module<T>::instance;

#if __cplusplus >= CPLUSPLUS_14

template <typename T, typename = void>
struct has_initializer : std::false_type {};

template <typename T>
struct has_initializer<T, decltype(&T::initialize, void())> : std::true_type {};

template <typename T>
typename std::enable_if<has_initializer<T>::value>::type initialize_instance(T* obj)
{
    obj->initialize();
}

template <typename T>
typename std::enable_if<!has_initializer<T>::value>::type initialize_instance(T*)
{
}

template <typename T, typename = void>
struct has_finalizer : std::false_type {};

template <typename T>
struct has_finalizer<T, decltype(&T::finalize, void())> : std::true_type {};

template <typename T>
typename std::enable_if<has_finalizer<T>::value>::type finalize_instance(T* obj)
{
    obj->finalize();
}

template <typename T>
typename std::enable_if<!has_finalizer<T>::value>::type finalize_instance(T*)
{
}

#else

inline void initialize_instance(void*)
{
    std::cerr << "unsupported module initializer" << std::endl;
}

inline void finalize_instance(void*)
{
    std::cerr << "unsupported module finalizer" << std::endl;
}

#endif

template <typename T>
void Module<T>::init()
{
    if (instance == 0) {
        instance = new T();
        initialize_instance(instance);
#ifdef _DEBUG
        std::string module_name = STL_util::class_name_of(instance);
        std::cout << "testing " << module_name << std::endl;
        T::test();
        std::cout << "testing complete" << std::endl;
#endif
    }
    instance->retain();
}

template <typename T>
void Module<T>::dispose()
{
    if (!instance)
        return;
    finalize_instance(instance);
    if (instance->release() == 0)
        instance = 0;
}

//
// class Base_module
//

template <typename T>
void Base_module::register_abstract_class()
{
    Class_registry* registry = module.instance->get_class_registry();
    registry->register_class(new Abstract_class<T>());
}

template <typename T>
void Base_module::register_class()
{
    Class_registry* registry = module.instance->get_class_registry();
    registry->register_class(new Class<T>());
}

template <typename T>
void Base_module::unregister_class()
{
    Class_registry* registry = module.instance->get_class_registry();
    Serializable_class* clazz = registry->lookup_class(T::class_id);
    registry->unregister_class(clazz);
}

}}

#endif
