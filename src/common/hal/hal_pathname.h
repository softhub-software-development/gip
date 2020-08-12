
//
//  hal_pathname.h
//
//  Created by Christian Lehner on 5/25/16.
//  Copyright (c) 2019 Softhub. All rights reserved.
//

#ifndef SOFTHUB_HAL_PATHNAME_H
#define SOFTHUB_HAL_PATHNAME_H

#include <base/base.h>
#ifdef __OBJC__
#import <Foundation/Foundation.h>
#endif

namespace SOFTHUB {
namespace HAL {

FORWARD_CLASS(Pathname);

//
// class Pathname, TODO: replace File_path
//

class Pathname : public BASE::Object<> {

    static const byte drive_mask = 0x1f;
    static const byte abs_bit = 0x80;
    static const byte dir_bit = 0x40;

    byte extra;
    BASE::String_vector path;
    std::string base;
    std::string ext;

    std::string to_string(char sep) const;

    static void parse_windows_path(const std::string& filepath, BASE::String_vector& path, std::string& base, std::string& ext, byte& extra);
    static void parse_path(const std::string& filepath, size_t head, BASE::String_vector& path, std::string& base, std::string& ext, byte& extra);
    static char map_drive_letter(byte extra);

public:
    Pathname() : extra(0) {}

    char get_drive_letter() const;
    bool is_absolute() const { return (extra & abs_bit) != 0; }
    bool is_directory() const { return (extra & dir_bit) != 0; }
    const BASE::String_vector& get_path() const { return path; }
    const std::string& get_basename() const { return base; }
    const std::string& get_extension() const { return ext; }
    void concat(const std::string& filename);

    void serialize(BASE::Serializer* serializer) const;
    void deserialize(BASE::Deserializer* deserializer);
    bool operator==(const Interface& obj) const;
    size_t hash() const;

    std::string to_string() const;
    std::string to_platform_string() const;
    std::string to_windows_string() const;
    std::string to_unix_string() const;

    static Pathname* from_string(const std::string& filepath);
    static Pathname* from_platform_string(const std::string& filepath);
    static Pathname* from_windows_string(const std::string& filepath);
    static Pathname* from_unix_string(const std::string& filepath);
#ifdef __OBJC__
    static Pathname* from_url(NSURL* url);
#endif
    static void parse_windows_path(const std::string& filepath, char& drive, BASE::String_vector& path, std::string& base, std::string& ext, bool& is_abs, bool& is_dir);
    static void parse_unix_path(const std::string& filepath, BASE::String_vector& path, std::string& base, std::string& ext, bool& is_abs, bool& is_dir);
    static void parse_basename(const std::string& filepath, std::string& base);

    static const char* any_separator;
    static const char windows_separator;
    static const char unix_separator;
    static const char separator;

    DECLARE_CLASS('spth');
};

}}

#endif
