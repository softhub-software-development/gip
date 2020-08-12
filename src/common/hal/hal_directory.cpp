
//
//  hal_directory.cpp
//
//  Created by Christian Lehner on 11/18/15.
//  Copyright (c) 2019 Softhub. All rights reserved.
//

#include "stdafx.h"
#include "hal_directory.h"
#include "hal_filesystem.h"
#include "hal_utils.h"
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <fcntl.h>
#ifdef PLATFORM_WIN
#include "hal_directory_win.h"
#else
#include <errno.h>
#include <unistd.h>
#define O_RAW 0
#endif
#ifdef PLATFORM_MAC
#include <sys/mount.h>
#elif defined(PLATFORM_UNIX)
#include <fstab.h>
#endif

using namespace SOFTHUB::BASE;
using namespace std;

namespace SOFTHUB {
namespace HAL {

//
// class Directory
//

bool Directory::create(mode_t mode)
{
#ifdef PLATFORM_WIN
    return ::_mkdir(path.c_str()) == 0;
#else
    return ::mkdir(path.c_str(), mode) == 0;
#endif
}

bool Directory::for_each_file(Directory_functor& functor)
{
    DIR* dir = ::opendir(path.c_str());
    if (!dir)
        return false;
    bool success = true;
    struct dirent* entry;
    while (success && !functor.is_interrupted() && (entry = ::readdir(dir)) != 0) {
        if (functor.is_valid(entry))
            success = functor(this, entry);
    }
    ::closedir(dir);
    return success;
}

bool Directory::remove_all_files()
{
    DIR* dir = ::opendir(path.c_str());
    if (!dir)
        return false;
    bool success = true;
    struct dirent* entry;
    while ((entry = ::readdir(dir)) != 0) {
        string filename = entry->d_name;
        if (filename != "." && filename != "..") {
            string file_path = File_path::concat(path, filename);
            success &= File_path::remove_file(file_path);
        }
    }
    success &= ::closedir(dir) == 0;
    return success;
}

bool Directory::remove_file(const string& filename)
{
    string filepath = File_path::concat(path, filename);
    return File_path::remove_file(filepath);
}

bool Directory::copy(Directory_copy_functor& fun)
{
    Directory directory(*this);
    bool success = fun.make_directory(fun.get_dst_dir());
    if (success) {
        do {
            success = directory.for_each_file(fun);
        } while (success && fun.next_level(directory));
    }
    return success;
}

bool Directory::copy(const string& dst_dir)
{
    Directory_copy_functor fun(path, dst_dir);
    return copy(fun);
}

bool Directory::clone(const string& dst_dir)
{
    Directory_clone_functor fun(path, dst_dir);
    return copy(fun);
}

int Directory::count(Directory_count_functor& fun, bool recursive)
{
    Directory directory(*this);
    bool success;
    do {
        success = directory.for_each_file(fun);
    } while (success && recursive && fun.next_level(directory));
    return fun.count;
}

int Directory::count(bool recursive)
{
    Directory_count_functor fun;
    return count(fun, recursive);
}

bool Directory::remove()
{
#if EXPERIMENTAL
    Directory directory(*this);
    Directory_remove_functor fun(path);
    bool success;
    do {
        success = directory.for_each_file(fun);
    } while (success && fun.next_level(directory));
    return success;
#else
    assert(!"TODO");
    return false;
#endif
}

int Directory::stat(const std::string& filepath, struct stat* st)
{
#ifdef PLATFORM_WIN
    int result = ::stat(filepath.c_str(), st);
#else
    int result = ::lstat(filepath.c_str(), st);
#endif
    return result;
}

#ifndef PLATFORM_WIN

bool Directory::resolve_sym_link(string& path, struct stat* st)
{
    // TODO: double check this
    String_set trail;
    char buf[PATH_MAX+1];
    do {
        if (trail.contains(path))   // check for circular links
            return false;
        int result = ::lstat(path.c_str(), st);
        if (result != 0)
            return false;
        if (S_ISLNK(st->st_mode)) {
            trail.insert(path);
            ssize_t len = ::readlink(path.c_str(), buf, PATH_MAX);
            buf[len] = '\0';
            if (len > 0 && buf[0] == '/') {
                path = buf;
            } else {
                string base = File_path::basepath_of(path);
                if (base.length() > 0)
                    path = File_path::concat(base, buf);
                else
                    path = buf;
            }
        }
    } while (S_ISLNK(st->st_mode) && trail.size() < 1000);
    return true;
}

#endif

//
// class Directory_deep_functor
//

Directory_deep_functor::Directory_deep_functor() :
    num_directories_processed(0), num_files_processed(0), num_failures(0), interrupted(false)
{
}

bool Directory_deep_functor::operator()(const Directory* directory, struct dirent* entry)
{
    string filename = entry->d_name;
    if (filename == "." || filename == "..")
        return true;
    bool success = true;
    string filepath = File_path::concat(directory->get_path(), filename);
    struct stat st;
    int result = Directory::stat(filepath, &st);
    if (result != 0) {
        num_failures++;
        return true;
    }
#ifndef PLATFORM_WIN
    if (S_ISLNK(st.st_mode))
        return true;
#endif
    if (S_ISDIR(st.st_mode)) {
#ifdef PLATFORM_WIN
        struct _finddata_t info;
        string tmp = filepath + "\\*";
        handle_type handle = (handle_type) _findfirst(tmp.c_str(), &info);
        if (handle >= 0)
            _findclose(handle);
        if ((st.st_mode & _S_IREAD) == 0 || handle < 0) {
#else
        if ((st.st_mode & S_IROTH) == 0 || (st.st_mode & S_IXOTH) == 0) {
#endif
            // do not attempt to recurse into protected directories
            if (!on_access_denied(filepath))
                return false;
            num_failures++;
        } else {
            success = process_directory(filepath);
            num_directories_processed++;
        }
    } else {
        success = process_file(filepath);
        num_files_processed++;
    }
    return success;
}

bool Directory_deep_functor::next_level(Directory& directory)
{
    if (dir_stack.empty())
        return false;
    directory = dir_stack.front();
    dir_stack.pop_front();
    return true;
}

//
// class Directory_count_functor
//

bool Directory_count_functor::process_directory(const string& src)
{
    Directory sub_dir(src);
    if (!dir_stack.contains(sub_dir))
        dir_stack.push_back(sub_dir);
    return true;
}

bool Directory_count_functor::process_file(const string& src)
{
    count++;
    return true;
}

//
// class Directory_copy_functor
//

Directory_copy_functor::Directory_copy_functor(const string& base_dir, const string& dst_dir) :
    base_dir(base_dir), dst_dir(dst_dir)
{
}

bool Directory_copy_functor::process_directory(const string& src)
{
    bool success = true;
    size_t src_dir_len = base_dir.length();
    string dst = dst_dir + src.substr(src_dir_len);
    Directory sub_dir(src);
    if (!dir_stack.contains(sub_dir)) {
        dir_stack.push_back(sub_dir);
        success = make_directory(dst);
        if (success)
            num_directories_processed++;
    }
    return success;
}

bool Directory_copy_functor::process_file(const string& src)
{
    size_t src_dir_len = base_dir.length();
    string dst = dst_dir + src.substr(src_dir_len);
    return copy_file(src, dst);
}

bool Directory_copy_functor::on_access_denied(const string& src)
{
    return true;
}

bool Directory_copy_functor::make_directory(const string& path)
{
    bool create_flag = false;
    return File_path::make_directory(path, create_flag);
}

bool Directory_copy_functor::copy_file(const string& src, const string& dst)
{
    return File_path::copy_file(src, dst);
}

//
// class Directory_clone_functor
//

Directory_clone_functor::Directory_clone_functor(const string& base_dir, const string& dst_dir) :
    Directory_copy_functor(base_dir, dst_dir)
{
}

bool Directory_clone_functor::make_directory(const string& path)
{
    bool create_flag = false;
    bool success = File_path::make_directory(path, create_flag);
    if (create_flag) {
        // set modification date
        File_info base_info(base_dir);
        File_time base_mod_time;
        bool success = base_info.read_file_modification_time(base_mod_time);
        if (!success)
            return false;
        File_info dst_info(path);
        File_time dst_mod_time;
        success = dst_info.write_file_modification_time(dst_mod_time);
    }
    return success;
}

bool Directory_clone_functor::copy_file(const string& src, const string& dst)
{
    return File_path::clone_file(src, dst);
}

//
// class Directory_remove_functor
//

bool Directory_remove_functor::process_directory(const string& filepath)
{
    assert(!"TODO");
    bool success = true;
    Directory sub_dir(filepath);
    if (!dir_stack.contains(sub_dir)) {
        dir_stack.push_back(sub_dir);
        // TODO
        num_directories_processed++;
    }
    return success;
}

bool Directory_remove_functor::process_file(const string& filepath)
{
    return File_path::remove_file(filepath);
}

bool Directory_remove_functor::on_access_denied(const string& src)
{
    return true;
}

}}
