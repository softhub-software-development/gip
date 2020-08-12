
//
//  hal_module.cpp
//
//  Created by Christian Lehner on 5/25/16.
//  Copyright (c) 2019 Softhub. All rights reserved.
//

#include "stdafx.h"
#include "hal_module.h"
#include "hal_utils.h"
#if defined PLATFORM_WIN
#include <objbase.h>
#elif defined PLATFORM_MAC
#include <dlfcn.h>
#elif defined PLATFORM_LINUX
#include <unistd.h>
#include <limits.h>
#endif

namespace SOFTHUB {
namespace HAL {

using namespace SOFTHUB::BASE;
using namespace std;

BASE::Module<Hal_module> Hal_module::module;

#if defined PLATFORM_MAC
void proxy_for_module() {}
#endif

//
// class Hal_module
//

Hal_module::Hal_module()
{
#if defined PLATFORM_WIN
    HRESULT res = CoInitializeEx(0, COINIT_APARTMENTTHREADED);  // Note: SHBrowseForFolder fails if initialized with COINIT_MULTITHREADED
    //assert(!FAILED(res));
    res = CoInitializeSecurity(0, -1, 0, 0, RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE, 0, EOAC_NONE, 0);
    assert(!FAILED(res));
    TCHAR buf[MAX_PATH];
	if (GetModuleFileName(0, buf, MAX_PATH))
		module_file_name = encode_utf8(buf);
#elif defined PLATFORM_MAC
    Dl_info info;
    if (dladdr((void*) proxy_for_module, &info))
        module_file_name = info.dli_fname;
#elif defined PLATFORM_IOS
    // TODO
#elif defined PLATFORM_LINUX
    char buf[PATH_MAX];
    ssize_t len = ::readlink("/proc/self/exe", buf, sizeof(buf)-1);
    if (len > 0) {
        buf[0] = '\0';
        module_file_name = buf;
    }
#else
#error unknown platform
#endif
    Base_module::module.init();
    Base_module::register_class<Pathname>();
#if FEATURE_HAL_THREAD_POOL
    thread_pool = new Thread_pool(max_number_of_pool_threads);
#endif
#if FEATURE_HAL_USB_DISCOVERY
    USB_discovery::initialize();
#endif
}

Hal_module::~Hal_module()
{
#if FEATURE_HAL_USB_DISCOVERY
    USB_discovery::finalize();
#endif
#if FEATURE_HAL_THREAD_POOL
    delete thread_pool;
#endif
    Base_module::unregister_class<Pathname>();
    Base_module::module.dispose();
#ifdef PLATFORM_WIN
    CoUninitialize();
#endif
}

void Hal_module::configure(IConfig* config)
{
#if FEATURE_HAL_THREAD_POOL
    int thread_pool_size = config->get_parameter("thread_pool_size", max_number_of_pool_threads);
    thread_pool->resize(thread_pool_size);
#endif
}

#ifdef _DEBUG

class Test_target : public BASE::Object<Runnable> {

    const char* name;
    int sleep_count;

public:
    Test_target() : name("default-target"), sleep_count(800) {}
    Test_target(const char* name, int delay) : name(name), sleep_count(delay) {}

    void fail(const std::exception& ex) {}

    void run()
    {
        log_message(INFO, "start pool thread");
        Thread::sleep(sleep_count);
        log_message(INFO, name);
    }
};

static void test_hal_mem_leak()
{
#if MEM_LEAK_CHECK
    dbg_init();
#endif
    Mutex mutex;
    for (int i = 0; i < 100; i++) {
        Lock::Block lock(mutex);
    }
#if MEM_LEAK_CHECK
    dbg_statistics();
    std::cout << "test_hal_mem_leak: " << dbg_leak_count() << std::endl;
    assert(dbg_leak_count() == 0);
#endif
}

static void test_pathnames()
{
    Pathname_ref pn1 = Pathname::from_string("/a/b/c.d");
    string ps1 = pn1->to_unix_string();
    assert(ps1 == "/a/b/c.d");
    Pathname_ref pn2 = Pathname::from_string("c.d");
    string ps2 = pn2->to_unix_string();
    assert(ps2 == "c.d");
    Pathname_ref pn3 = Pathname::from_string("a/b/c.d");
    string ps3 = pn3->to_unix_string();
    assert(ps3 == "a/b/c.d");
    Pathname_ref pn4 = Pathname::from_string("a/b/.d");
    string ps4 = pn4->to_unix_string();
    assert(ps4 == "a/b/.d");
    Pathname_ref pn5 = Pathname::from_windows_string("c:\\a\\b/c.d");
    string ps5 = pn5->to_unix_string();
    assert(ps5 == "c:/a/b/c.d");
    Pathname_ref pn6 = Pathname::from_windows_string("c:a\\b.d");
    string ps6 = pn6->to_windows_string();
    assert(ps6 == "c:a\\b.d");
    Pathname_ref pn7 = Pathname::from_string("a/b.d/");
    string ps7 = pn7->to_unix_string();
    assert(ps7 == "a/b.d/" && !pn7->is_absolute() && pn7->is_directory());
}

static void test_strings()
{
    string s = "a b";
    String_vector v;
    Strings::split(s, v);
    assert(v.size() == 2 && v[0] == "a" && v[1] == "b");
    string s1 = "abc.def";
    string s2 = "abc";
    string s3 = ".def";
    string s4 = ".de";
    assert(Strings::starts_with(s1, s2));
    assert(Strings::ends_with(s1, s3));
    assert(!Strings::ends_with(s1, s4));
}

class Test_notification_target : public BASE::Object<INotification> {

public:
    void on_observation(IObserver* observer)
    {
        cout << "kill switch pressed" << endl;
    }
};

static void test_observer()
{
    INotification_ref target(new Test_notification_target());
    IObserver_ref observer(new Kill_switch_observer(target));
    observer->observe();
}

static void test_usb()
{
#if FEATURE_HAL_USB_DISCOVERY
    USB_discovery::instance->start();
#endif
}

static void test_utils()
{
    string a, b, c;
    os_version(a, b, c);
    cout << "os version " << a << ", " << b << ", " << c << endl;
}

void Hal_module::test()
{
    string name = full_user_name();
    printf("user name: %s\n", name.c_str());
#ifdef UNIT_TEST
    File_path test_file("/tmp/");
    test_file.ensure("test", "foo bla");
    Directory test_dir("/tmp");
    bool success = test_dir.remove_file("test");
    assert(success);
    Reference<Test_target> test_target = new Test_target;
    Thread* thread = new Thread((Runnable*) test_target);
    thread->start();
    Thread::sleep(1000);
    module.instance->thread_pool->run(new Test_target("A", 10));
    module.instance->thread_pool->run(new Test_target("B", 20));
    module.instance->thread_pool->run(new Test_target("C", 40));
    module.instance->thread_pool->run(new Test_target("D", 80));
    module.instance->thread_pool->run(new Test_target("E", 10));
    module.instance->thread_pool->run(new Test_target("F", 200));
    module.instance->thread_pool->run(new Test_target("G", 10));
    module.instance->thread_pool->run(new Test_target("H", 10));
    module.instance->thread_pool->run(new Test_target("I", 10));
#endif
#ifdef PLATFORM_WIN
//  Thread_pool pool(4);
//  pool.run(test_target);
    string p0 = File_path::make_directory_path("\\foo");
    assert(p0 == "\\foo\\");
    string serno;
//  bool res = File_system::read_serial_number("\\\\.\\PhysicalDrive0", serno);
    Volume_info vol_info;
    File_system fs("C:\\");
    bool res = fs.read_volume_info(vol_info);
#else
    string p0 = File_path::make_directory_path("/foo");
    assert(p0 == "/foo/");
    Directory tmp("/Users/chris/tmp");
//  tmp.copy("/Users/chris/tmp-copy-test");
    string s0 = File_path::basename_of("/a/b");
    string s1 = File_path::basename_of("b.c");
    string s2 = File_path::basename_of("/a/b.c");
    assert(s0 == "b" && s1 == "b" && s2 == "b");
    string path, base, ext;
    bool is_dir;
    File_path::parse("/a/b.c", path, base, ext, is_dir);
    assert(path == "/a" && base == "b" && ext == "c");
    int ntmp = tmp.count();
#endif
    Semaphore sem(3);
    test_pathnames();
//  test_hal_mem_leak();
    string cwd = File_path::get_cwd();
    assert(!cwd.empty());
    test_strings();
//  test_observer();
    test_usb();
    test_utils();
}

#endif

}}
