
//
//  hal_pathname.cpp
//
//  Created by Christian Lehner on 5/25/16.
//  Copyright (c) 2019 Softhub. All rights reserved.
//

#include "stdafx.h"
#include "hal_pathname.h"

// TODO

using namespace SOFTHUB::BASE;
using namespace std;

namespace SOFTHUB {
namespace HAL {

//
// class Pathname, TODO: replace File_path
//

char Pathname::get_drive_letter() const
{
    return map_drive_letter(extra);
}

void Pathname::concat(const string& filename)
{
    path.append(filename);
}

void Pathname::serialize(Serializer* serializer) const
{
    serializer->write(extra);
    STL_util::serialize(serializer, path);
    serializer->write(base);
    serializer->write(ext);
}

void Pathname::deserialize(Deserializer* deserializer)
{
    deserializer->read(extra);
    STL_util::deserialize(deserializer, path);
    deserializer->read(base);
    deserializer->read(ext);
}

bool Pathname::operator==(const Interface& obj) const
{
    const Pathname* other = dynamic_cast<const Pathname*>(&obj);
    assert(other);
    return this == other || (extra == other->extra && path == other->path && base == other->base && ext == other->ext);
}

size_t Pathname::hash() const
{
    return extra + STL_util::hash(path) + std::hash<string>()(base) + std::hash<string>()(ext);
}

string Pathname::to_string() const
{
    return to_unix_string();
}

string Pathname::to_platform_string() const
{
#ifdef PLATFORM_WIN
    return to_windows_string();
#else
    return to_unix_string();
#endif
}

string Pathname::to_windows_string() const
{
    return to_string(windows_separator);
}

string Pathname::to_unix_string() const
{
    return to_string(unix_separator);
}

string Pathname::to_string(char sep) const
{
    stringstream stream;
    char drive = get_drive_letter();
    if (drive)
        stream << drive << ":";
    if (is_absolute())
        stream << sep;
    for (size_t i = 0, n = path.size(); i < n; i++)
        stream << path[i] << sep;
    if (!base.empty())
        stream << base;
    if (!ext.empty())
        stream << "." << ext;
    if (is_directory())
        stream << sep;
    return stream.str();
}

Pathname* Pathname::from_string(const string& filepath)
{
    return from_unix_string(filepath);
}

Pathname* Pathname::from_platform_string(const string& filepath)
{
#ifdef PLATFORM_WIN
    return from_windows_string(filepath);
#else
    return from_unix_string(filepath);
#endif
}

Pathname* Pathname::from_windows_string(const string& filepath)
{
    Pathname* result = new Pathname();
    parse_windows_path(filepath, result->path, result->base, result->ext, result->extra);
    return result;
}

Pathname* Pathname::from_unix_string(const string& filepath)
{
    Pathname* result = new Pathname();
    parse_path(filepath, 0, result->path, result->base, result->ext, result->extra);
    return result;
}

void Pathname::parse_windows_path(const string& filepath, String_vector& path, string& base, string& ext, byte& extra)
{
    size_t head;
    size_t col_pos = filepath.find_first_of(":");
    if (col_pos == 1) {
        head = 2;
        extra |= (tolower(filepath[0]) - 'a' + 1) & drive_mask;
    } else {
        head = 0;
        extra = 0;
    }
    parse_path(filepath, head, path, base, ext, extra);
}

void Pathname::parse_path(const string& filepath, size_t head, String_vector& path, string& base, string& ext, byte& extra)
{
    size_t dot_pos = filepath.find_last_of(".");
    size_t sep_pos = filepath.find_first_of(any_separator);
    size_t dir_pos = filepath.find_last_of(any_separator);
    if (dir_pos == filepath.length() - 1)
        extra |= dir_bit;
    if (sep_pos == string::npos) {
        if (dot_pos == string::npos) {
            base = filepath;
        } else {
            base = filepath.substr(head, dot_pos - head);
            ext = filepath.substr(dot_pos + 1);
        }
    } else {
        if (sep_pos == head) {
            extra |= abs_bit;
            head++;
        }
        while ((sep_pos = filepath.find_first_of(any_separator, head)) < dir_pos) {
            string element = filepath.substr(head, sep_pos - head);
            if (!element.empty())
                path.append(element);
            head = sep_pos + 1;
        }
    }
    if (dot_pos == string::npos) {
        base = filepath.substr(head);
    } else {
        base = filepath.substr(head, dot_pos - head);
        if (dir_pos == string::npos)
            ext = filepath.substr(dot_pos + 1);
        else
            ext = filepath.substr(dot_pos + 1, dir_pos - dot_pos - 1);
    }
}

void Pathname::parse_windows_path(const string& filepath, char& drive, String_vector& path, string& base, string& ext, bool& is_abs, bool& is_dir)
{
    byte extra = 0;
    parse_windows_path(filepath, path, base, ext, (byte&) drive);
    drive = map_drive_letter(extra);
    is_abs = (extra & abs_bit) != 0;
    is_dir = (extra & dir_bit) != 0;
}

void Pathname::parse_unix_path(const string& filepath, String_vector& path, string& base, string& ext, bool& is_abs, bool& is_dir)
{
    byte extra = 0;
    parse_path(filepath, 0, path, base, ext, extra);
    is_abs = (extra & abs_bit) != 0;
    is_dir = (extra & dir_bit) != 0;
}

void Pathname::parse_basename(const std::string& filepath, std::string& base)
{
    String_vector path;
    string ext;
    byte extra = 0;
    parse_path(filepath, 0, path, base, ext, extra);
}

char Pathname::map_drive_letter(byte extra)
{
    byte value = extra & drive_mask;
    return value ? value + 'a' - 1 : 0;
}

const char* Pathname::any_separator = "\\/";
const char Pathname::windows_separator = '\\';
const char Pathname::unix_separator = '/';
#ifdef PLATFORM_WIN
const char Pathname::separator = windows_separator;
#else
const char Pathname::separator = unix_separator;
#endif

}}
