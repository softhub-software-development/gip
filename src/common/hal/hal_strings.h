
//
//  hal_strings.h
//
//  Created by Christian Lehner on 01/01/17.
//  Copyright (c) 2019 Softhub. All rights reserved.
//

#ifndef HAL_STRINGS_H
#define HAL_STRINGS_H

#include <base/base.h>

namespace SOFTHUB {
namespace HAL {

#define HEX(x) (0 <= (x) && (x) <= 9 ? (x) + '0' : (x) - 10 + 'a')
#define ORD(x) ('0' <= (x) && (x) <= '9' ? (x) - '0' : (x) - 'a' + 10)

//
// class Strings
//

class Strings {

public:
    static int utf8_to_wchar(wchar_t* wc, const char* str, size_t len);
#ifdef PLATFORM_WIN
    static void trim(std::string& str, const std::string& blanks = "\t\n\v\f\r ");
    static void trim_left(std::string& str, const std::string& blanks = "\t\n\v\f\r ");
    static void trim_right(std::string& str, const std::string& blanks = "\t\n\v\f\r ");
#else
    static void trim(std::string& s, int is(int) = isspace);
    static void trim_left(std::string& s, int is(int) = isspace);
    static void trim_right(std::string& s, int is(int) = isspace);
#endif
    static long atoi(const std::string& s);
    static void to_upper(std::string& s);
    static void to_lower(std::string& s);
    static void split(const std::string& s, BASE::String_vector& tokens, const std::string& sep = " ,\t", bool include_empty_tokens = false);
    static bool starts_with(const std::string& s, const char c);
    static bool ends_with(const std::string& s, const char c);
    static bool starts_with(const std::string& s, const std::string& tail);
    static bool ends_with(const std::string& s, const std::string& tail);
    static bool match_string(const std::string& str, const BASE::String_vector& sv);
    static bool match_string_ext(const std::string& str, const BASE::String_vector& sv);
    static bool is_all_capitals(const std::string& s);
    static bool is_punctuation(wchar_t c);
    static bool is_space(wchar_t c);
    static bool is_hyphen(wchar_t c);
};

}}

#endif
