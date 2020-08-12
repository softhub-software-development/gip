
//
//  util_string.h
//
//  Created by Christian Lehner on 4/22/15.
//  Copyright (c) 2019 Softhub. All rights reserved.
//

#ifndef LIB_UTIL_STRING_H
#define LIB_UTIL_STRING_H

#include <base/base.h>

namespace SOFTHUB {
namespace UTIL {

class Substring;
class Snippet;
class String_util;
class String_state;

typedef BASE::Vector<Substring> Substring_vector;

//
// class Substring
//

class Substring {

    std::string text;
    size_t pos, len;

public:
    Substring(const std::string& text, size_t pos, size_t len) : text(text), pos(pos), len(len) {}

    const std::string& get_text() const { return text; }
    std::string get_substring() const { return text.substr(pos, len); }
    size_t get_position() const { return pos; }
    size_t length() const { return len; }
};

//
// class Snippet
//

class Snippet {

    short toi_pos;      // position of toi (text of interest)
    short toi_len;      // length of toi (text of interest)
    short buf_len;      // length of the buffer
    char* buf;          // buffer holding len elements
    bool cut_off_head;  // ... text
    bool cut_off_tail;  // text ...

public:
    Snippet(short len);
    ~Snippet();

    size_t get_toi_position() const { return toi_pos; }
    size_t get_toi_length() const { return toi_len; }
    std::string get_text() const { return buf; }
    bool is_cut_off_head() const { return cut_off_head; }
    bool is_cut_off_tail() const { return cut_off_tail; }
    void snip(const std::string& text, long ioi = 0, long loi = 0);
    void save(BASE::Serializer& serializer) const;
    void restore(BASE::Deserializer& deserializer);
};

//
// class String_util
//

class String_util {

    static char soundex_transform(char c);

public:
    static std::string limit_to_length(const std::string& s, size_t n, const std::string& tail_marker = "_");
    static std::string limit_to_length(const std::string& s, size_t n, size_t head, size_t tail);
    static void trim(std::string& s, int is(int) = isspace);
    static void trim_left(std::string& s, int is(int) = isspace);
    static void trim_right(std::string& s, int is(int) = isspace);
    static size_t to_upper(std::string& s, size_t idx = 0, size_t cnt = std::string::npos);
    static size_t to_lower(std::string& s, size_t idx = 0, size_t cnt = std::string::npos);
    static size_t find_case_insensitive(const std::string& s, const std::string& sub);
    static size_t substitute_umlauts(std::string& s, size_t idx = 0, size_t cnt = std::string::npos);
    static size_t substring(std::string& s, size_t idx = 0, size_t cnt = std::string::npos);
    static std::string escape(const std::string& s, char q, char e = '\\');
    static void split(const std::string& s, BASE::String_vector& tokens, const std::string& sep = " ,\t", bool include_empty_tokens = false);
    static void split(const std::string& s, Substring_vector& tokens, const std::string& sep = " ,\t", bool include_empty_tokens = false);
    static std::string join(const BASE::String_vector& v, int max_count = 0, const std::string& sep = ",");
    static std::string join(const BASE::String_set& set, int max_count = 0, const std::string& sep = ",");
    static void base64_encode(const char* s, int count, char* result);
    static void base64_decode(const char* s, int count, char* result);
    static std::string base64_encode(const std::string& s);
    static std::string base64_decode(const std::string& s);
    static void html_encode(std::string& s);
    static void html_decode(std::string& s);
    static void html_markup(std::string& text, const std::string& phrase, const std::string& delim, const std::string& begin_tag, const std::string& end_tag);
    static std::string num_to_string(size_t num);
    static std::string num_to_string(size_t num, int exp);
    static std::string hash(const std::string& s, int modulo);
    static int levenshtein_distance(const std::string& s, const std::string& t);
    static std::string soundex(const std::string& s);
#if FEATURE_NET_SSL
    static void md5(const std::string& s, unsigned char digest[16]);
#endif
};

//
// class String_state
//

class String_state {

    std::string* str;

protected:
    void initialize();
    void finalize();
    size_t scan(std::string& s, int start);
    void stop_scan();

public:
    String_state();
    ~String_state();

    size_t to_lower_case(std::string& s, size_t idx, size_t cnt);
    size_t to_upper_case(std::string& s, size_t idx, size_t cnt);
    size_t substitute_umlauts(std::string& s, size_t idx, size_t cnt);
    size_t substring(std::string& s, size_t idx, size_t cnt);
    size_t next(const std::string& s, size_t idx);
    bool operator<<(char c);
    bool operator<<(const char* s);

    size_t head, byte_index, byte_count, char_index, char_count;
    void* scanner;
};

//
// class String_iterator
//

class String_iterator {

    const std::string& str;
    mutable size_t offset;
    mutable String_state state;

public:
    String_iterator(const std::string& str);

    unsigned next() const;
};

}}

#endif
