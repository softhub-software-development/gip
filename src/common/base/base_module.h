
//
//  base_module.h
//
//  Created by Softhub.
//  Copyright (c) 2019 Softhub. All rights reserved.
//

#ifndef BASE_MODULE_H
#define BASE_MODULE_H

#include "base_platform.h"
#include "base_class_registry.h"
#include "base_options.h"
#include <istream>
#include <ostream>
#include <sstream>

#ifdef UNUSED
#elif defined(__GNUC__)
#define UNUSED(x) UNUSED_ ## x __attribute__((unused))
#elif defined(__LCLINT__)
#define UNUSED(x) /*@unused@*/ x
#elif defined(__cplusplus)
#define UNUSED(x)
#else
#define UNUSED(x) x
#endif

namespace SOFTHUB {
namespace BASE {

//
// class Module
//

template <typename T>
class Module : Object<Interface> {

public:
    ~Module() {}

    static void init();
    static void dispose();

    static T* instance;
};

//
// class Base_module
//

class Base_module : public Object<> {

    Class_registry* class_registry;

    static IConfig_ref configuration;

public:
    Base_module();
    ~Base_module();

    Class_registry* get_class_registry() { return class_registry; }
    static IConfig* get_configuration() { return configuration; }

    static bool setup(int argc, char* argv[], const char* options, IConfig* config = 0);
    template <typename T> static void register_abstract_class();
    template <typename T> static void register_class();
    template <typename T> static void unregister_class();
    static void unregister_all_classes();
    static void print_mem(std::ostream& stream);
    static void print_trace(int depth);
    static Module<Base_module> module;
#ifdef _DEBUG
    static void test();
#endif
};

}}

#include "base_module_inline.h"

#endif
