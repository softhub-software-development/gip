
/*
 *  util_module.cpp
 *
 *  Created by Christian Lehner on 8/17/10.
 *  Copyright 2018 softhub. All rights reserved.
 */

#include "stdafx.h"
#include "util.h"
#include <iostream>
#include <fstream>
#ifdef _DEBUG
#ifndef PLATFORM_WIN
#include <sys/time.h>
#endif
#endif

using namespace SOFTHUB::BASE;
using namespace SOFTHUB::HAL;
using namespace std;

namespace SOFTHUB {
namespace UTIL {

using namespace BASE;

BASE::Module<Util_module> Util_module::module;

Util_module::Util_module()
{
    Base_module::module.init();
#if FEATURE_UTIL_ALL
    Base_module::register_class<Metric_units>();
    Base_module::register_class<Imperial_units>();
    Base_module::register_class<Nautical_units>();
#ifndef PLATFORM_WIN
    // TODO: fails in hash map
    Units::init();
#endif
#endif
}

Util_module::~Util_module()
{
#if FEATURE_UTIL_ALL
    // TODO: Units::dispose()
    Base_module::unregister_class<Nautical_units>();
    Base_module::unregister_class<Imperial_units>();
    Base_module::unregister_class<Metric_units>();
#endif
    Base_module::module.dispose();
}

#ifdef _DEBUG

static void test_string_utils()
{
#if FEATURE_UTIL_ALL && FEATURE_HAL_UTILS
#ifndef PLATFORM_WIN
    HAL::Timing timing;
    string m = "München";
    string t = m;
    String_util::html_encode(t);
    String_util::html_decode(t);
    assert(m == t);
    string text = "Some text (description)";
    String_util::html_markup(text, "Some text", " \t\n", "<b>", "</b>");
    assert(text == "<b>Some</b> <b>text</b> (description)");
    string mucup = "MÜNCHEN";
    string muc = mucup;
    String_util::to_lower(muc);
    String_util::to_upper(muc);
    assert(mucup == muc);
    timing.begin();
    for (int i = 0; i < 1000; i++) {
        string hello = "Hällo WÖrld!";
        String_util::to_lower(hello);
        assert(hello == "hällo wörld!");
    }
    long msec = timing.end();
    assert(msec < 1000);
    string hello2 = "Hällo WÖrlD!";
    size_t hello2_len = String_util::to_lower(hello2, 7, 2);
    assert(hello2_len == 3 && hello2 == "Hällo wörlD!");
    string subtext1 = "a荒bcd";
    size_t sublen = subtext1.length();
    size_t nsub1 = String_util::substring(subtext1, 1, 2);
    assert(sublen == 7 && nsub1 == 4);
    string subtext2 = "a荒bcd";
    size_t nsub2 = String_util::substring(subtext2);
    assert(sublen == 7 && nsub2 == 7);
    String_vector sv;
    String_util::split("a;", sv, ";", false);
    assert(sv.size() == 1 && sv[0].length() == 1);
    sv.clear();
    String_util::split("a;", sv, ";", true);
    assert(sv.size() == 2 && sv[1].empty());
    timing.begin();
    int d0 = String_util::levenshtein_distance("amberg", "amberg");
    assert(d0 == 0);
    int d1 = String_util::levenshtein_distance("amberg", "bamberg");
    assert(d1 == 1);
    int d2 = String_util::levenshtein_distance("amberg", "hamburg");
    assert(d2 == 2);
    int d3 = String_util::levenshtein_distance("amberg", "düsseldorf");
    assert(d3 > 2);
    string s1 = String_util::soundex("anonymous");
    string s2 = String_util::soundex("annonymus");
    assert(s1 == s2);
    string s3 = String_util::soundex("obama");
    string s4 = String_util::soundex("osama");
    assert(s3 != s4);
    msec = timing.end();
    assert(msec < 100);
    timing.begin();
    int d4 = String_util::levenshtein_distance("browserspezifikation", "brecher");
    assert(d4 > 2);
    msec = timing.end();
    assert(msec < 100);
    byte* src = new byte[8192];
    byte* dst0 = 0;
    byte* dst1 = new byte[8192];
    long dst0_len = 0;
    long dst1_len = 8192;
    strcpy((char*) src, "abc");
    timing.begin();
    int cres = Compression::compress(src, 8192, dst0, dst0_len);
    int dres = Compression::decompress(dst0, dst0_len, dst1, dst1_len);
    assert(cres == 0 && dres == 0);
    msec = timing.end();
    assert(msec < 1000);
    assert(strcmp((char*) src, (char*) dst1) == 0);
    delete[] dst1;
    delete[] dst0;
    delete[] src;
#endif
#endif
}

static void test_tmp_base64()
{
#if 0
    const char* s = "Hello World";
    char enc[50], dec[50];
    String_util::base64_encode(s, (int) strlen(s), enc);
    assert(strcmp(enc, "SGVsbG8gV29ybGQ=") == 0);
    String_util::base64_decode(enc, (int) strlen(enc), dec);
    string senc = base64_encode((const unsigned char*) s, (unsigned) strlen(s));
    assert(strcmp(senc.c_str(), "SGVsbG8gV29ybGQ=") == 0);
    string sdec = base64_decode(senc);
    assert(strcmp(s, sdec.c_str()) == 0);
    string sdec2 = String_util::base64_decode(senc);
    assert(strcmp(s, sdec2.c_str()) == 0);
#endif
}

static void test_base64()
{
    cout << "test_base64" << endl;
    const char* s = "Hello World and some more words to make message longer.";
    string senc = String_util::base64_encode(s);
    string sdec = String_util::base64_decode(senc);
    assert(strcmp(s, sdec.c_str()) == 0 && sdec == sdec);
    const char* s2 = "MIICXgIBAAKBgQDUk9mVnx4frkdiU2zDoiUSPOdtY4lkCbXMQBLv7rBO1QpiDP2q\n";
    string senc2 = String_util::base64_encode(s2);
    string sdec2 = String_util::base64_decode(senc2);
    assert(strcmp(s2, sdec2.c_str()) == 0 && sdec2 == sdec2);
}

static void test_sha()
{
#if FEATURE_NET_SSL
    cout << "test_sha" << endl;
    Crypt crypt;
    string s1, s2, s3;
    crypt.sha256("test", s1);
    crypt.sha256("test", s2);
    crypt.sha256("text", s3);
    assert(s1 == s2 && s2 != s3);
    byte b1[] = { 1, 2, 3 };
    unique_byte_ptr p;
    size_t p_size = 0;
    crypt.sha256(b1, sizeof(b1), p, p_size);
    assert(p_size > 0 && p);
#endif
}

FORWARD_CLASS(Log_consumer);

class Log_consumer : public BASE::Object<IFile_consumer> {

public:
    Log_consumer() {}

    bool consumer_process(const String_vector& columns) {
        return true;
    }

    void consumer_reset() {}
};

static void test_log_reader()
{
    IConfig* config = Base_module::module.instance->get_configuration();
    if (!config)
        return;
    string test_data_dir = config->get_test_data_directory();
    string filepath = File_path::concat(test_data_dir, "test.log");
    Log_consumer_ref consumer(new Log_consumer());
    File_observer_ref observer(new File_observer(consumer));
    observer->tail(filepath, false);
}

static void test_string_iterator()
{
    const string& str = "abüc";
    String_iterator it(str);
    unsigned c;
    while ((c = it.next()) != 0) {
        cout << "c: " << c << std::endl;
    }
}

void Util_module::test()
{
    test_string_iterator();
    test_log_reader();
    test_string_utils();
    test_tmp_base64();
    test_base64();
#if FEATURE_NET_SSL
    test_sha();
#endif
}

#endif

}}
