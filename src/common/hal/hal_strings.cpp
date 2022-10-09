
//
//  hal_strings.cpp
//
//  Created by Christian Lehner on 01/01/17.
//  Copyright (c) 2019 Softhub. All rights reserved.
//

#include "stdafx.h"
#include "hal_strings.h"
#include "base/base_platform.h"
#ifdef PLATFORM_WIN
#include <windows.h>
#include <functional>
#endif

using namespace SOFTHUB::BASE;
using namespace std;

namespace SOFTHUB {
namespace HAL {

int Strings::utf8_to_wchar(wchar_t* wc, const char* str, size_t len)
{
#ifdef WIN32
    BYTE c = *str;
    int clen = 0 <= c && c < 128 ? 1 : 2;
    int n = MultiByteToWideChar(CP_UTF8, 0, str, clen, wc, (int) len);
    return clen;
#else
    return mbtowc(wc, str, len);
#endif
}

#ifdef PLATFORM_WIN

void Strings::trim(string& s, const string& blanks)
{
    trim_left(s, blanks);
    trim_right(s, blanks);
}

void Strings::trim_left(string& str, const string& chars)
{
    str.erase(0, str.find_first_not_of(chars));
}

void Strings::trim_right(string& str, const string& chars)
{
    str.erase(str.find_last_not_of(chars) + 1);
}

#else

void Strings::trim(string& s, int is(int))
{
    trim_left(s, is);
    trim_right(s, is);
}

void Strings::trim_left(string& s, int is(int))
{
#if __cplusplus >= CPLUSPLUS14
    s.erase(s.begin(), find_if(s.begin(), s.end(), [is](int c) { return !is(c); }));
#else
    s.erase(s.begin(), find_if(s.begin(), s.end(), not1(ptr_fun<int,int>(is))));
#endif
}

void Strings::trim_right(string& s, int is(int))
{
#if __cplusplus >= CPLUSPLUS14
    s.erase(find_if(s.rbegin(), s.rend(), [is](int c) { return !is(c); }).base(), s.end());
#else
    s.erase(find_if(s.rbegin(), s.rend(), not1(ptr_fun<int,int>(is))).base(), s.end());
#endif
}

#endif

void Strings::split(const string& tmp, String_vector& tokens, const string& sep, bool include_empty_tokens)
{
    string s = tmp; // TODO: why do we need to copy?
    size_t len, i = 0, n = s.length();
    while (true) {
        size_t j = s.find_first_of(sep, i);
        if (j == string::npos) {
            len = n - i;
            if (include_empty_tokens || len > 0) {
                const string& sub = s.substr(i, len);
                tokens.append(sub);
            }
            break;
        }
        len = j - i;
        if (include_empty_tokens || len > 0) {
            const string& sub = s.substr(i, len);
            tokens.append(sub);
        }
        i = j + 1;
    }
}

long Strings::atoi(const string& s)
{
    return ::atoi(s.c_str());
}

void Strings::to_upper(string& s)
{
    transform(s.begin(), s.end(), s.begin(), ::toupper);
}

void Strings::to_lower(string& s)
{
    transform(s.begin(), s.end(), s.begin(), ::tolower);
}

bool Strings::starts_with(const string& s, const char c)
{
    size_t n = s.length();
    return n > 0 && s[0] == c;
}

bool Strings::ends_with(const string& s, const char c)
{
    size_t n = s.length();
    return n > 0 && s[n-1] == c;
}

bool Strings::starts_with(const string& s, const string& tail)
{
    size_t slen = s.length();
    size_t tlen = tail.length();
    if (slen < tlen)
        return false;
    return s.compare(0, tlen, tail) == 0;
}

bool Strings::ends_with(const string& s, const string& tail)
{
    size_t slen = s.length();
    size_t tlen = tail.length();
    if (slen < tlen)
        return false;
    return s.compare(slen - tlen, tlen, tail) == 0;
}

bool Strings::match_string(const string& str, const String_vector& sv)
{
    for (size_t i = 0, n = sv.size(); i < n; i++) {
        if (str.find(sv[i]) != string::npos)
            return true;
    }
    return false;
}

bool Strings::match_string_ext(const string& str, const String_vector& sv)
{
    for (size_t i = 0, n = sv.size(); i < n; i++) {
        if (Strings::ends_with(str, sv[i]))
            return true;
    }
    return false;
}

bool Strings::is_all_capitals(const string& s)
{
    for (size_t i = 0, n = s.length(); i < n; i++) {
        char c = s[i];
        if (!isalpha(c))
            return false;
        if (islower(c))
            return false;
    }
    return true;
}

bool Strings::is_punctuation(wchar_t c)
{
    return c == '.' || c == ',' || c == ';' || c == ':' || c == '-';
}

bool Strings::is_space(wchar_t c)
{
    return c == ' ' || c == '\n' || c == '\r' || c == '\t';
}

bool Strings::is_hyphen(wchar_t c)
{
    return c == 0x00ad;
}

}}
