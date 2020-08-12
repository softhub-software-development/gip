
//
//  hal_filesystem.h
//
//  Created by Christian Lehner on 11/18/15.
//  Copyright (c) 2019 Softhub. All rights reserved.
//

#ifndef SOFTHUB_HAL_FILESYSTEM_H
#define SOFTHUB_HAL_FILESYSTEM_H

#include <base/base.h>
#include <sys/stat.h>
#ifdef PLATFORM_WIN
#include <windows.h>
#include <io.h>
#include <stdlib.h>
#include <string.h>
#include <direct.h>
#else
#include <time.h>
#include <utime.h>
#include <sys/stat.h>
#endif

#ifdef PLATFORM_WIN
#ifdef WINDOWS_XP_COMPATIBLE
typedef FILETIME File_time;
#define FILE_MOD_TIME_EQUAL(a, b)   ((a).dwLowDateTime == (b).dwLowDateTime && (a).dwHighDateTime == (b).dwHighDateTime)
#else
typedef FILE_BASIC_INFO File_time;
#define FILE_MOD_TIME_EQUAL(a, b)   ((a).ChangeTime.QuadPart == (b).ChangeTime.QuadPart)
#endif
#else
typedef struct utimbuf File_time;
#define FILE_MOD_TIME_EQUAL(a, b)   ((a).modtime == (b).modtime)
#endif

namespace SOFTHUB {
namespace HAL {

//
// class File_path
//

class File_path {

    std::string path;

    static bool ensure_tail(const std::string& filename, bool& create_flag, mode_t mode = 0777);

public:
    File_path(const std::string& path) : path(path) {}

    bool ensure(mode_t mode = 0777);
    bool ensure(const std::string& sub_path, mode_t mode = 0777);
    bool ensure(const std::string& sub_path, bool& create_flag, mode_t mode = 0777);
    bool ensure(const std::string& filename, const std::string& content, mode_t mode = 0644);
    std::string concat(const std::string& filename);
    std::string get_base_path() const;
    std::string get_filename() const;
    std::string get_extension() const;

    static bool make_directory(const std::string& path, bool& create_flag, mode_t mode = 0755);
    static bool copy_file(const std::string& src, const std::string& dst);
    static bool clone_file(const std::string& src, const std::string& dst);
    static bool remove_file(const std::string& filepath);
    static bool rename_file(const std::string& src, const std::string& dst);
    static bool exists(const std::string& filepath);
    static bool unique_filepath(std::string& filepath);
    static void parse(const std::string& filepath, std::string& path, std::string& basename, std::string& ext, bool& is_dir);
    static std::string make_directory_path(const std::string& path);
    static std::string basepath_of(const std::string& filepath);
    static std::string filename_of(const std::string& filepath);
    static std::string basename_of(const std::string& filepath);
    static std::string lastname_of(const std::string& filepath);
    static std::string extension_of(const std::string& filepath);
    static std::string concat(const std::string& path, const std::string& filename);
    static std::string user_home();
    static std::string get_cwd();
#ifdef PLATFORM_WIN
    static bool special_path(int cid, std::string& path, bool create_flag = false);
#else
    static bool special_path(const std::string& base, const std::string& filename, std::string& path, bool create_flag = false);
#ifdef PLATFORM_MAC
    static bool app_support_path(std::string& path, bool create_flag = false);
#endif
#endif
    static bool app_data_path(std::string& path, bool create_flag = false);
    static bool my_documents_path(std::string& path, bool create_flag = false);
    static bool my_pictures_path(std::string& path, bool create_flag = false);
    static bool my_music_path(std::string& path, bool create_flag = false);
    static bool my_video_path(std::string& path, bool create_flag = false);
    static bool common_documents_path(std::string& path, bool create_flag = false);
    static bool common_pictures_path(std::string& path, bool create_flag = false);
    static bool common_music_path(std::string& path, bool create_flag = false);
    static bool common_video_path(std::string& path, bool create_flag = false);
    static bool ensure_dir(const std::string& path, mode_t mode = 0777);
    static bool ensure_dir(const std::string& path, const std::string& sub_path, bool& create_flag, mode_t mode = 0777);
    static bool ensure_dir(const std::string& path, const std::string& filename, const std::string& content, mode_t mode = 0644);

    static const char separator;
};

//
// class File_info
//

class File_info {

    std::string filepath;

public:
    File_info(const std::string& filepath) : filepath(filepath) {}

    bool read_file_characteristics(File_time& mod_time, large& size);
    bool read_file_modification_time(File_time& mod_time);
    bool write_file_modification_time(const File_time& mod_time);
    large get_file_size() const;
    int is_regular_file() const;
    int is_directory() const;
    int is_link() const;
};

//
// class Volume_info
//

class Volume_info {

    friend class File_system;

    std::string logical_name;
    std::string physical_name;
    std::string fs_type;
    std::string mount;

public:
    const std::string& get_logical_name() const { return logical_name; }
    const std::string& get_physical_name() const { return physical_name; }
    const std::string& get_fs_type() const { return fs_type; }
    const std::string& get_mount() const { return mount; }
};

//
// class File_system
//

class File_system {

    std::string volume;

public:
    File_system(const std::string& volume) : volume(volume) {}

    bool read_volume_info(Volume_info& info);

    static void mounted_filesystems(BASE::String_vector& volumes);
    static void mounted_volumes(BASE::String_vector& volumes);
    static std::string logical_volume_name(const std::string& name);
    static bool read_serial_number(const std::string& device, std::string& serno);
};

}}

//
// null stream
//

template<typename Ch, typename Traits = std::char_traits<Ch> >
struct basic_nullbuf : std::basic_streambuf<Ch, Traits> {
     typedef std::basic_streambuf<Ch, Traits> base_type;
     typedef typename base_type::int_type int_type;
     typedef typename base_type::traits_type traits_type;

     virtual int_type overflow(int_type c) {
         return traits_type::not_eof(c);
     }
};

typedef basic_nullbuf<char> nullbuf;
typedef basic_nullbuf<wchar_t> wnullbuf;

extern std::ostream cnull;
extern std::wostream wcnull;

#endif
