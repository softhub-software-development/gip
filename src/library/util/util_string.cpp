
//
//  util_string.cpp
//
//  Created by Christian Lehner on 4/22/15.
//  Copyright (c) 2019 Softhub. All rights reserved.
//

#include "stdafx.h"
#include "util_string.h"
#include <sstream>
#include <algorithm>
#include <iterator>
#include <functional>
#include <cctype>
#include <locale>
#include <iostream>
#include <iomanip>
#include <string>
#if FEATURE_NET_SSL
#include <openssl/md5.h>
#endif
#ifdef PLATFORM_WIN
#include <windows.h>
#endif

using namespace SOFTHUB::BASE;

namespace SOFTHUB {
namespace UTIL {

static bool is_numeric(const std::string& s);

//
// class Snippet
//

Snippet::Snippet(short len) : toi_pos(0), toi_len(0), buf_len(len), cut_off_head(false), cut_off_tail(false)
{
    buf = new char[len];
    ::memset(buf, 0, len * sizeof(char));
}

Snippet::~Snippet()
{
    delete[] buf;
}

void Snippet::snip(const std::string& text, long ioi, long loi)
{
    assert(toi_pos == 0 && toi_len == 0);
    long n = (long) text.length();
    long i0 = ioi - buf_len / 4;
    long i = std::max(0L, i0);
    long jn = i + buf_len - 1;
    long j = std::min(jn, n);
    const char* s = text.c_str();
    long snip_len = std::max(0L, j - i);
    assert(0 <= snip_len && snip_len < buf_len);
#ifdef PLATFORM_WIN
    ::strncpy_s(buf, buf_len, &s[i], snip_len);
#else
    ::strncpy(buf, &s[i], snip_len);
#endif
    buf[snip_len] = '\0';
    toi_pos = (short) (ioi - i);
    toi_len = (short) loi;
    cut_off_head = i0 > 0;
    cut_off_tail = jn < n;
}

void Snippet::save(Serializer& serializer) const
{
    serializer.write(toi_pos);
    serializer.write(toi_len);
    serializer.write(buf_len);
    for (short i = 0; i < buf_len; i++)
        serializer.write(buf[i]);
    serializer.write(cut_off_head);
    serializer.write(cut_off_tail);
}

void Snippet::restore(Deserializer& deserializer)
{
    deserializer.read(toi_pos);
    deserializer.read(toi_len);
    short ext_len = 0;
    deserializer.read(ext_len);
    assert(buf_len >= ext_len);
    ext_len = std::min(buf_len, ext_len);
    for (short i = 0; i < ext_len; i++)
        deserializer.read(buf[i]);
    deserializer.read(cut_off_head);
    deserializer.read(cut_off_tail);
}

//
// class String_util
//

std::string String_util::limit_to_length(const std::string& s, size_t n, const std::string& tail_marker)
{
    size_t slen = s.length();
    size_t nlen = n - tail_marker.length();
    assert(slen >= nlen);
    return slen <= n ? s : s.substr(0, nlen) + tail_marker;
}

std::string String_util::limit_to_length(const std::string& s, size_t n, size_t head, size_t tail)
{
    size_t len = s.length();
    if (len <= n)
        return s;
    std::string result = s.substr(0, head);
    result += "...";
    result += s.substr(len - tail, tail);
    return result;
}

void String_util::trim(std::string& s, int is(int))
{
    trim_left(s, is);
    trim_right(s, is);
}

void String_util::trim_left(std::string& s, int is(int))
{
#if __cplusplus >= CPLUSPLUS14
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [is](int c) { return !is(c); }));
#else
    s.erase(s.begin(), find_if(s.begin(), s.end(), not1(std::ptr_fun<int,int>(is))));
#endif
}

void String_util::trim_right(std::string& s, int is(int))
{
#if __cplusplus >= CPLUSPLUS14
    s.erase(std::find_if(s.rbegin(), s.rend(), [is](int c) { return !is(c); }).base(), s.end());
#else
    s.erase(find_if(s.rbegin(), s.rend(), not1(std::ptr_fun<int,int>(is))).base(), s.end());
#endif
}

size_t String_util::to_upper(std::string& s, size_t idx, size_t cnt)
{
    String_state state;
    return state.to_upper_case(s, idx, cnt);
}

size_t String_util::to_lower(std::string& s, size_t idx, size_t cnt)
{
    String_state state;
    return state.to_lower_case(s, idx, cnt);
}

size_t String_util::find_case_insensitive(const std::string& s, const std::string& sub)
{
    std::string s_lower = s;
    to_lower(s_lower);
    std::string sub_lower = sub;
    to_lower(sub_lower);
    return s_lower.find(sub_lower);
}

size_t String_util::substitute_umlauts(std::string& s, size_t idx, size_t cnt)
{
    String_state state;
    return state.substitute_umlauts(s, idx, cnt);
}

size_t String_util::substring(std::string& s, size_t idx, size_t cnt)
{
    String_state state;
    return state.substring(s, idx, cnt);
}

std::string String_util::escape(const std::string& s, char q, char e)
{
    std::stringstream stream;
    for (size_t i = 0; i < s.length(); i++) {
        char c = s[i];
        if (c == q)
            stream << e << e;
        stream << c;
    }
    return stream.str();
}

void String_util::split(const std::string& s, String_vector& tokens, const std::string& sep, bool include_empty_tokens)
{
    size_t i = 0, n = s.length();
    while (true) {
        size_t j = s.find_first_of(sep, i);
        if (j == std::string::npos) {
            size_t len = n - i;
            if (include_empty_tokens || len > 0) {
                std::string sub = s.substr(i, len);
                tokens.append(sub);
            }
            break;
        }
        size_t len = j - i;
        if (include_empty_tokens || len > 0) {
            std::string sub = s.substr(i, len);
            tokens.append(sub);
        }
        i = j + 1;
    }
}

void String_util::split(const std::string& s, Substring_vector& tokens, const std::string& sep, bool include_empty_tokens)
{
    size_t i = 0, n = s.length();
    while (true) {
        size_t j = s.find_first_of(sep, i);
        if (j == std::string::npos) {
            size_t len = n - i;
            if (include_empty_tokens || len > 0) {
                Substring sub(s, i, len);
                tokens.append(sub);
            }
            break;
        }
        size_t len = j - i;
        if (include_empty_tokens || len > 0) {
            Substring sub(s, i, len);
            tokens.append(sub);
        }
        i = j + 1;
    }
}

std::string String_util::join(const String_vector& v, int max_count, const std::string& sep)
{
    std::string result;
    int count = 0;
    String_vector::const_iterator it = v.begin();
    String_vector::const_iterator tail = v.end();
    if (it != tail) {
        result += *it++;
        while (it != tail && (max_count <= 0 || count++ < max_count)) {
            result += ", ";
            result += *it++;
        }
    }
    return result;
}

std::string String_util::join(const String_set& set, int max_count, const std::string& sep)
{
    std::string result;
    int count = 0;
    String_set::const_iterator it = set.begin();
    String_set::const_iterator tail = set.end();
    if (it != tail) {
        result += *it++;
        while (it != tail && (max_count <= 0 || count++ < max_count)) {
            result += ", ";
            result += *it++;
        }
    }
    return result;
}

static char base64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

#define BASE64_ENCODE(x)    (base64_chars[x])
#define BASE64_DECODE(x)    ('A' <= (x) && (x) <= 'Z' ? (x) - 'A' : \
                            ('a' <= (x) && (x) <= 'z' ? (x) - 'a' + 26 : \
                            ('0' <= (x) && (x) <= '9' ? (x) - '0' + 52 : \
                            ('=' == (x) ? 0 : \
                            ('+' == (x) ? 62 : \
                            ('/' == (x) ? 63 : (assert(!"invalid base64 character"), 0)))))))

void String_util::base64_encode(const char* s, int count, char* result)
{
    int i, j;
    int r = count % 3;
    int n = count > r ? count - r : 0;
    for (i = 0, j = 0; i < n; i += 3, j += 4) {
        byte s0 = s[i];
        byte s1 = s[i+1];
        byte s2 = s[i+2];
        result[j+0] = BASE64_ENCODE((s0 >> 2) & 0x3f);
        result[j+1] = BASE64_ENCODE((s0 << 4 | s1 >> 4) & 0x3f);
        result[j+2] = BASE64_ENCODE((s1 << 2 | s2 >> 6) & 0x3f);
        result[j+3] = BASE64_ENCODE(s2 & 0x3f);
        assert(0 <= result[j+0] && result[j+0] < 64);
        assert(0 <= result[j+1] && result[j+1] < 64);
        assert(0 <= result[j+2] && result[j+2] < 64);
        assert(0 <= result[j+3] && result[j+3] < 64);
    }
    if (r == 1) {
        byte s0 = s[i];
        byte s1 = 0;
        result[j+0] = BASE64_ENCODE((s0 >> 2) & 0x3f);
        result[j+1] = BASE64_ENCODE((s0 << 4 | s1 >> 4) & 0x3f);
        result[j+2] = '=';
        result[j+3] = '=';
        j += 4;
    } else if (r == 2) {
        byte s0 = s[i];
        byte s1 = s[i+1];
        byte s2 = 0;
        result[j+0] = BASE64_ENCODE((s0 >> 2) & 0x3f);
        result[j+1] = BASE64_ENCODE((s0 << 4 | s1 >> 4) & 0x3f);
        result[j+2] = BASE64_ENCODE((s1 << 2 | s2 >> 6) & 0x3f);
        result[j+3] = '=';
        j += 4;
    }
    result[j] = '\0';
}

void String_util::base64_decode(const char* s, int count, char* result)
{
    int j = 0;
    for (int i = 0; i < count; i += 4) {
        byte e0 = BASE64_DECODE(s[i]);
        byte e1 = BASE64_DECODE(s[i+1]);
        byte e2 = BASE64_DECODE(s[i+2]);
        byte e3 = BASE64_DECODE(s[i+3]);
        result[j++] = (e0 << 2) | ((e1 >> 4) & 0x3f);
        if (e1) {
            result[j++] = (e1 << 4) | ((e2 >> 2) & 0x3f);
            if (e2)
                result[j++] = (e2 << 6) | (e3 & 0x3f);
        }
    }
    result[j] = '\0';
}

std::string String_util::base64_encode(const std::string& s)
{
    std::stringbuf buf;
    size_t i, len = s.length();
    size_t r = len % 3;
    size_t n = len > r ? len - r : 0;
    for (i = 0; i < n; i += 3) {
        byte s0 = s[i];
        byte s1 = s[i+1];
        byte s2 = s[i+2];
        buf.sputc(BASE64_ENCODE((s0 >> 2) & 0x3f));
        buf.sputc(BASE64_ENCODE((s0 << 4 | s1 >> 4) & 0x3f));
        buf.sputc(BASE64_ENCODE((s1 << 2 | s2 >> 6) & 0x3f));
        buf.sputc(BASE64_ENCODE(s2 & 0x3f));
    }
    if (r == 1) {
        byte s0 = s[n];
        byte s1 = 0;
        buf.sputc(BASE64_ENCODE((s0 >> 2) & 0x3f));
        buf.sputc(BASE64_ENCODE((s0 << 4 | s1 >> 4) & 0x3f));
        buf.sputc('=');
        buf.sputc('=');
    } else if (r == 2) {
        byte s0 = s[n];
        byte s1 = s[n+1];
        byte s2 = 0;
        buf.sputc(BASE64_ENCODE((s0 >> 2) & 0x3f));
        buf.sputc(BASE64_ENCODE((s0 << 4 | s1 >> 4) & 0x3f));
        buf.sputc(BASE64_ENCODE((s1 << 2 | s2 >> 6) & 0x3f));
        buf.sputc('=');
    }
    return buf.str();
}

std::string String_util::base64_decode(const std::string& s)
{
    std::stringbuf buf;
    for (size_t i = 0, n = s.length(); i < n; i += 4) {
        assert(i+3 < n);
        char s0 = s[i];
        char s1 = s[i+1];
        char s2 = s[i+2];
        char s3 = s[i+3];
        byte e0 = BASE64_DECODE(s0);
        byte e1 = BASE64_DECODE(s1);
        byte e2 = BASE64_DECODE(s2);
        byte e3 = BASE64_DECODE(s3);
        buf.sputc((e0 << 2) | ((e1 >> 4) & 0x3f));
        if (s2 != '=') {
            buf.sputc((e1 << 4) | ((e2 >> 2) & 0x3f));
            if (s3 != '=')
                buf.sputc((e2 << 6) | (e3 & 0x3f));
        }
    }
    return buf.str();
}

void String_util::html_encode(std::string& s)
{
    // TODO: incomplete implementation
    std::stringstream stream;
    std::string::const_iterator it = s.begin();
    std::string::const_iterator tail = s.end();
    while (it != tail) {
        char u0 = *it++;
        int v0 = u0 & 0xff;
        if (0xC0 <= v0 && v0 <= 0xDF && it != tail) {
            int u1 = *it++;
            int v1 = u1 & 0xff;
            switch (v0) {
            case 0xC2:
                stream << "&#" << v1 << ";";
                break;
            case 0xC3:
                stream << "&#" << (v1 + 0x40) << ";";
                break;
            case 0xC4:
                stream << "&#" << (v1 + 0x80) << ";";
                break;
            case 0xC5:
                stream << "&#" << (v1 + 0xC0) << ";";
                break;
            default:
                stream << u0 << u1;
                break;
            }
        } else {
            stream << u0;
        }
    }
    s = stream.str();
}

void String_util::html_decode(std::string& s)
{
    // TODO: incomplete implementation
    std::string result;
    std::string prefix = "&#";
    size_t a = 0, b;
    if ((b = s.find(prefix, a)) != std::string::npos) {
        std::stringstream stream;
        do {
            size_t c = s.find(';', b);
            if (c == std::string::npos || c - b > 6)
                break;
            std::string num_str = s.substr(b + 2, c - b - 2);
            if (!is_numeric(num_str))
                break;
            int num = atoi(num_str.c_str());
            if (num) {
                stream << s.substr(a, b - a);
                if (num < 0x80) {
                    stream << (char) num;
                } else if (num < 0xC0) {
                    stream << (char) 0xC2 << (char) num;
                } else if (num < 0x100) {
                    stream << (char) 0xC3 << (char) (num - 0x40);
                } else if (num < 0x140) {
                    stream << (char) 0xC4 << (char) (num - 0x80);
                } else if (num < 0x180) {
                    stream << (char) 0xC5 << (char) (num - 0xC0);
                } else {
                    // TODO
                    stream << (char) num;
                }
            }
            a = c + 1;
        } while ((b = s.find(prefix, a)) != std::string::npos);
        stream << s.substr(a);
        s = stream.str();
    }
}

#ifndef PLATFORM_WIN

void String_util::html_markup(std::string& text, const std::string& phrase, const std::string& delim, const std::string& begin_tag, const std::string& end_tag)
{
    std::stringstream stream;
    String_vector words;
    std::string lower_phrase = phrase;
    to_lower(lower_phrase);
    split(lower_phrase, words, " \t\n");
    Substring_vector tokens;
    split(text, tokens, delim);
    size_t a = 0, b;
    for (size_t i = 0, n = tokens.size(); i < n; i++) {
        Substring sub = tokens[i];
        std::string s = sub.get_substring();
        std::string lower_s = s;
        to_lower(lower_s);
        b = sub.get_position();
        std::string pre = text.substr(a, b - a);
        if (words.contains(lower_s)) {
            stream << pre << begin_tag << s << end_tag;
        } else {
            stream << pre << s;
        }
        a = b + sub.length();
    }
    text = stream.str();
}

#endif

std::string String_util::num_to_string(size_t num)
{
    std::stringstream stream;
    stream << num;
    return stream.str();
}

std::string String_util::num_to_string(size_t num, int exp)
{
    std::stringstream stream;
    stream << std::setw(exp) << std::setfill('0') << (num % (1 << exp));
    return stream.str();
}

std::string String_util::hash(const std::string& s, int modulo)
{
    size_t hash = std::hash<std::string>()(s) % modulo;
    std::stringstream stream;
    stream << std::setw(3) << std::setfill('0') << hash;
    return stream.str();
}

int String_util::levenshtein_distance(const std::string& s1, const std::string& s2)
{
    int len1 = (int) s1.size(), len2 = (int) s2.size();
    std::vector<int> col(len2+1), prev_col(len2+1);
    for (int i = 0; i < (int) prev_col.size(); i++)
        prev_col[i] = i;
    for (int i = 0; i < len1; i++) {
        col[0] = i+1;
        for (int j = 0; j < len2; j++)
            col[j+1] = std::min(std::min(prev_col[j+1] + 1, col[j] + 1), prev_col[j] + (s1[i] == s2[j] ? 0 : 1));
        col.swap(prev_col);
    }
    return prev_col[len2];
}

char String_util::soundex_transform(char c)
{
    static const std::string consonants[6] = { "bfpv", "cgjkqsxz", "dt", "l", "mn", "r" };
    for (int i = 0; i < 6; i++)
        if (consonants[i].find(c) != std::string::npos)
            return i + 1 + '0';
    return c;
}

std::string String_util::soundex(const std::string& s)
{
    std::string result;
    // validate s
#if __cplusplus >= CPLUSPLUS14
    if (std::find_if(s.begin(), s.end(), [](int c) { return !isalpha(c); }) != s.end())
        return result;
#else
    if (std::find_if(s.begin(), s.end(), not1(ptr_fun<int,int>(isalpha))) != s.end())
        return result;
#endif
    result.resize(s.length());
#if __cplusplus >= CPLUSPLUS14
    transform(s.begin(), s.end(), result.begin(), [](int c) { return tolower(c); });
#else
    transform(s.begin(), s.end(), result.begin(), ptr_fun<int,int>(tolower));
#endif
    // convert soundex letters to codes
    transform(result.begin() + 1, result.end(), result.begin() + 1, soundex_transform);
    // collapse adjacent identical digits
    result.erase(unique(result.begin() + 1, result.end()), result.end());
    // remove all non-digits following the first letter
#if __cplusplus >= CPLUSPLUS14
    result.erase(remove_if(result.begin() + 1, result.end(), [](int c) { return !isdigit(c); }), result.end());
#else
    result.erase(remove_if(result.begin() + 1, result.end(), not1(ptr_fun<int,int>(isdigit))), result.end());
#endif
    result += "000";
    result.resize(4);
    return result;
}

#if FEATURE_NET_SSL
void String_util::md5(const std::string& s, unsigned char digest[16])
{
    MD5_CTX ctx;
    MD5_Init(&ctx);
    MD5_Update(&ctx, s.c_str(), s.length());
    MD5_Final(digest, &ctx);
}
#endif

static bool is_numeric(const std::string& s)
{
    for (size_t i = 0, n = s.length(); i < n; i++)
        if (!isdigit(s[i]))
            return false;
    return true;
}

//
// class String_state
//

String_state::String_state() : str(0), head(0), byte_index(0), byte_count(0), char_index(0), char_count(0)
{
    initialize();
}

String_state::~String_state()
{
    finalize();
}

bool String_state::operator<<(char c)
{
    (*str)[byte_index++] = c;
    if (++char_index >= char_count)
        stop_scan();
    return char_index >= char_count;
}

bool String_state::operator<<(const char* s)
{
    for (int i = 0; s[i]; i++)
        (*str)[byte_index++] = s[i];
    if (++char_index >= char_count)
        stop_scan();
    return char_index >= char_count;
}

//
// class String_iterator
//

String_iterator::String_iterator(const std::string& str) : str(str), offset(0)
{
}

unsigned String_iterator::next() const
{
    size_t cnt = state.next(str, offset);
    size_t idx = offset;
    offset += cnt;
    switch (cnt) {
    case 1:
        return str[idx];
    case 2:
        return (str[idx] & 0xff) << 8 | (str[idx+1] & 0xff);
    case 3:
        return (str[idx] & 0xff) << 16 | (str[idx+1] & 0xff) << 8 | (str[idx+2] & 0xff);
    case 4:
        return (str[idx] & 0xff) << 24 | (str[idx+1] & 0xff) << 16 | (str[idx+2] & 0xff) << 8 | (str[idx+3] & 0xff);
    }
    return 0;
}

}}
