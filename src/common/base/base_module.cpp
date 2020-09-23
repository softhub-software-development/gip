
//
//  base_module.cpp
//
//  Created by Softhub.
//  Copyright (c) 2019 Softhub. All rights reserved.
//

#include "stdafx.h"
#include "base_module.h"
#include "base_class_registry.h"
#include "base_serialization.h"
#include "base_options.h"
#include "base_io.h"
#include <string>
#include <sstream>
#ifndef PLATFORM_WIN
#include <execinfo.h>
#include <unistd.h>
#endif
#include <signal.h>
#include <fstream>

using namespace std;

namespace SOFTHUB {
namespace BASE {

//
// signal handler to print stack trace on SIGSEGV
//

static void sig_segv_handler(int sig)
{
    Base_module::print_trace(100);
    exit(-1);
}

//
// class Base_module
//

Module<Base_module> Base_module::module;
IConfig_ref Base_module::configuration;

Base_module::Base_module() : class_registry(new Class_registry())
{
    signal(SIGSEGV, sig_segv_handler);
}

Base_module::~Base_module()
{
    delete class_registry;
}

bool Base_module::setup(int argc, char* argv[], const char* options, IConfig* config)
{
    configuration = config ? config : (IConfig*) new Configuration();
    return configuration->parse(argc, argv, options);
}

void Base_module::unregister_all_classes()
{
    Class_registry* registry = module.instance->get_class_registry();
    registry->unregister_all_classes();
}

void Base_module::print_mem(ostream& stream)
{
#ifndef PLATFORM_WIN
    int tsize = 0, resident = 0, share = 0;
    ifstream buffer("/proc/self/statm");
    buffer >> tsize >> resident >> share;
    buffer.close();
    long page_size_kb = sysconf(_SC_PAGE_SIZE) / 1024;
    double rss = resident * page_size_kb;
    double shared_mem = share * page_size_kb;
    stream << "rss: " << rss << " kb - ";
    stream << "shared memory: " << shared_mem << " kb - ";
    stream << "private memory: " << rss - shared_mem << " kb";
#endif
}

void Base_module::print_trace(int depth)
{
#ifndef PLATFORM_WIN
    log_message(ERR, "stack trace");
    void** array = new void*[depth];
    int size = backtrace(array, depth);
    char** strings = backtrace_symbols(array, size);
    stringstream stream;
    print_mem(stream);
    stream << " number of stack frames: " << size;
    string msg = stream.str();
    log_message(ERR, msg);
    for (int i = size - 1; i >= 0; i--) {
        stream.str("");
        stream << "[" << i << "] " << strings[i];
        msg = stream.str();
        log_message(ERR, msg);
    }
    delete[] strings;
    delete[] array;
#endif
}

#ifdef _DEBUG

class Test_class : public Object<> {

    int int_member;

public:
    Test_class(int m = 4242) : int_member(m) {}

    void serialize(Serializer* serializer) const
    {
        serializer->write(int_member);
    }

    void deserialize(Deserializer* deserializer)
    {
        deserializer->read(int_member);
    }

    bool operator<(const Interface& obj) const
    {
        const Test_class* other = dynamic_cast<const Test_class*>(&obj);
        assert(other);
        return other && int_member < other->int_member;
    }

    bool operator==(const Interface& obj) const
    {
        const Test_class* other = dynamic_cast<const Test_class*>(&obj);
        assert(other);
        return other && int_member == other->int_member;
    }

    DECLARE_CLASS('tstc');
};

FORWARD_CLASS(Test_class);
DECLARE_ARRAY(Test_class_ref, Test_array);

static void test_serialization()
{
    stringstream out(ios_base::out | ios_base::binary);
    Stream_serializer serializer(out);
    bool bool_val = 1;
    serializer.write(bool_val);
    char char_val = 'x';
    serializer.write(char_val);
    byte byte_val = 129;
    serializer.write(byte_val);
    short short_val = 42;
    serializer.write(short_val);
    int int_val = 123456789;
    serializer.write(int_val);
    long long_val = 1234567890;
    serializer.write(long_val);
    large large_val = 1234567890123456789LL;
    serializer.write(large_val);
    float float_val = 42.1f;
    serializer.write(float_val);
    double double_val = 12345678901234567890.123;
    serializer.write(double_val);
    std::string string_val = "abc";
    serializer.write(string_val);
    Test_class_ref null_ref_val;
    serializer.write(null_ref_val);
    Test_class_ref ref_val(new Test_class);
    serializer.write(ref_val);
    Reference<Array<Test_class_ref> > ref_null_array;
    serializer.write(ref_null_array);
    Reference<Array<Test_class_ref> > ref_array(new Array<Test_class_ref>(0));
    serializer.write(ref_array);
    serializer.write(ref_val);
    serializer.write(ref_array);
    Reference<Array<Test_class_const_ref> > const_ref_array(new Array<Test_class_const_ref>(0));
    serializer.write(const_ref_array);
    out.flush();

    string s = out.str();

    // read from string
    stringstream in(s, ios_base::in | ios_base::binary);
    Stream_deserializer deserializer(in);
    bool bool_result;
    deserializer.read(bool_result);
    assert(bool_result == bool_val);
    char char_result;
    deserializer.read(char_result);
    assert(char_result == char_val);
    byte byte_result;
    deserializer.read(byte_result);
    assert(byte_result == byte_val);
    short short_result;
    deserializer.read(short_result);
    assert(short_result == short_val);
    int int_result;
    deserializer.read(int_result);
    assert(int_result == int_val);
    long long_result;
    deserializer.read(long_result);
    assert(long_result == long_val);
    large large_result;
    deserializer.read(large_result);
    assert(large_result == large_val);
    float float_result;
    deserializer.read(float_result);
    assert(float_result == float_val);
    double double_result;
    deserializer.read(double_result);
    assert(double_result == double_val);
    std::string string_result;
    deserializer.read(string_result);
    assert(string_result == string_val);
    Test_class_ref null_ref_result;
    deserializer.read(null_ref_result);
    assert(null_ref_result == null_ref_val);
    Test_class_ref ref_result;
    deserializer.read(ref_result);
    assert(ref_result == ref_val);
    Reference<Array<Test_class_ref> > ref_null_array_result;
    deserializer.read(ref_null_array_result);
    assert(ref_null_array_result == ref_null_array);
    Reference<Array<Test_class_ref> > ref_array_result;
    deserializer.read(ref_array_result);
    assert(ref_array_result->equals(ref_array.get_pointer()));
    deserializer.read(ref_result);
    assert(ref_result == ref_val);
    deserializer.read(ref_array_result);
    assert(ref_array_result->equals(ref_array.get_pointer()));
    Reference<Array<Test_class_const_ref> > const_ref_array_result;
    deserializer.read(const_ref_array_result);
    assert(const_ref_array_result->equals(const_ref_array));
}

static void test_arrays()
{
    Array<Test_class_ref> array_a(10);
    Array<Test_class_ref> array_b(10);
    Test_class_ref array_test_ref;
    array_a[1] = array_test_ref;
    array_b[1] = array_test_ref;
    assert(array_a != array_b);         // two distinct array allocations
    assert(array_a.equals(&array_b));   // but the array elements are the same

    Test_array_ref test_array(new Test_array());
    for (int i = 0; i < 1000; i++) {
        test_array->append(new Test_class());
    }

    Test_array_ref test_array2(new Test_array());
    test_array2->append(new Test_class(1));
    test_array2->append(new Test_class(2));
    test_array2->append(new Test_class(3));
    Test_class_ref result2(new Test_class(2));
    Test_class_ref result4(new Test_class(4));
    bool found2 = test_array2->binary_search(result2);
    bool found4 = test_array2->binary_search(result4);
    assert(found2 && !found4);
}

static void test_containers()
{
    Test_class_ref element(new Test_class());
    Vector<Test_class_ref> v;
    v.append(new Test_class());
    assert(v.contains(element));

    Stack<string> stack;
    stack.push("foo");
    stack.push("bar");
    assert(stack.contains("foo"));
    assert(!stack.contains("bla"));

    Hash_array<string,Test_class_ref> assoc;
    assoc.insert("foo", element);
    Test_class_ref element2(new Test_class());
    assoc.insert("foo", element2);
    assert(assoc.get("foo") == element2);
    assoc.remove("foo");
    assert(assoc.empty());
}

static void test_config()
{
    IConfig_ref config(new Configuration());
    config->set_parameter("foo", "bar");
    config->set_parameter("foo", "bla");
    string str = config->get_parameter("foo", "blub");
    assert(str == "bla");
}

void Base_module::test()
{
    register_class<Test_class>();
    test_serialization();
    test_arrays();
    test_containers();

    Reference<> ref;
    Weak_reference<> wref;
    ref = wref;

#ifndef PLATFORM_WIN
    char* argv[7];
    argv[0] = (char*) 0;
    argv[1] = (char*) "-a";
    argv[2] = (char*) "-b";
    argv[3] = (char*) "argument1";
    argv[4] = (char*) "-b";
    argv[5] = (char*) "argument2";
    argv[6] = (char*) "-f";
    Configuration_ref configuration(new Configuration());
    configuration->parse(7, argv, "ab:f");
#endif

    test_config();

    IConfig* config = Base_module::module.instance->get_configuration();
    if (!config)
        return;
    string test_data_dir = config->get_test_data_directory();
    string filepath = test_data_dir + "test.html";

    FILE* file = fopen(filepath.c_str(), "r");
    //Stream_io<FILE> fio(file);
    if (file)
        fclose(file);

    fstream fs;
    fs.open(filepath);
    //Stream_io<> sio(fs);
    fs.close();
}

#endif

}}

void log_assert(const char* filename, int lineno, const char* func)
{
    const size_t buf_size = 1024;
    char buf[buf_size];
    snprintf(buf, buf_size, "assert failed %s, line %d, in function %s", filename, lineno, func);
    log_message(ERR, buf);
    SOFTHUB::BASE::Base_module::print_trace(100);
}
