
//
//  hal_directory.h
//
//  Created by Christian Lehner on 11/18/15.
//  Copyright (c) 2019 Softhub. All rights reserved.
//

#ifndef SOFTHUB_HAL_DIRECTORY_H
#define SOFTHUB_HAL_DIRECTORY_H

#ifndef EXPERIMENTAL
#define EXPERIMENTAL 0
#endif

#include <base/base.h>
#include <sys/stat.h>
#ifdef PLATFORM_WIN
#include "hal_directory_win.h"
#else
#include <dirent.h>
#endif

namespace SOFTHUB {
namespace HAL {

class Directory;
class Directory_functor;
class Directory_count_functor;
class Directory_copy_functor;
class Directory_clone_functor;
class Directory_remove_functor;

typedef BASE::List<Directory> Directory_stack;

//
// class Directory
//

class Directory {

    std::string path;

    bool copy(Directory_copy_functor& fun);
    int count(Directory_count_functor& fun, bool recursive);

public:
    Directory(const std::string& path) : path(path) {}

    const std::string& get_path() const { return path; }
    bool create(mode_t mode = 0755);
    bool remove();
    bool for_each_file(Directory_functor& functor);
    bool remove_file(const std::string& filename);
    bool remove_all_files();
    bool copy(const std::string& dst_dir);
    bool clone(const std::string& dst_dir);
    int count(bool recursive = false);

    bool operator==(const Directory& directory) const { return path == directory.path; }
    bool operator!=(const Directory& directory) const { return path != directory.path; }

    static int stat(const std::string& filepath, struct stat* st);
#ifndef PLATFORM_WIN
    static bool resolve_sym_link(std::string& path, struct stat* st);
#endif
};

//
// interface Directory_functor
//

class Directory_functor {

public:
    virtual bool is_interrupted() const = 0;
    virtual bool is_valid(struct dirent* entry) const = 0;
    virtual bool operator()(const Directory* directory, struct dirent* entry) = 0;
};

//
// interface Directory_flat_functor
//

class Directory_flat_functor : public Directory_functor {

    bool interrupted;

public:
    Directory_flat_functor() : interrupted(false) {}

    bool is_interrupted() const { return interrupted; }
    void interrupt() { interrupted = true; }
    bool operator()(const Directory* directory, struct dirent* entry) { return !interrupted; }
};

//
// class Directory_deep_functor
//

class Directory_deep_functor : public Directory_functor {

protected:
    Directory_stack dir_stack;
    int num_directories_processed;
    int num_files_processed;
    int num_failures;
    bool interrupted;

    virtual bool process_directory(const std::string& filepath) = 0;
    virtual bool process_file(const std::string& filepath) = 0;
    virtual bool on_access_denied(const std::string& filepath) = 0;

public:
    Directory_deep_functor();

    bool is_interrupted() const { return interrupted; }
    void interrupt() { interrupted = true; }
    bool next_level(Directory& directory);
    bool is_valid(struct dirent* entry) const { return true; }
    bool operator()(const Directory* directory, struct dirent* entry);
};

//
// class Directory_count_functor
//

class Directory_count_functor : public Directory_deep_functor {

    friend class Directory;

protected:
    int count;

    bool process_directory(const std::string& filepath);
    bool process_file(const std::string& filepath);
    bool on_access_denied(const std::string& filepath) { return true; }

public:
    Directory_count_functor() : count(0) {}
};

//
// class Directory_copy_functor
//

class Directory_copy_functor : public Directory_deep_functor {

protected:
    std::string base_dir;
    std::string dst_dir;

    bool process_directory(const std::string& filepath);
    bool process_file(const std::string& filepath);
    bool on_access_denied(const std::string& filepath);

public:
    Directory_copy_functor(const std::string& base_dir, const std::string& dst_dir);

    const std::string& get_base_dir() { return base_dir; }
    const std::string& get_dst_dir() { return dst_dir; }

    virtual bool make_directory(const std::string& path);
    virtual bool copy_file(const std::string& src, const std::string& dst);
};

//
// class Directory_clone_functor
//

class Directory_clone_functor : public Directory_copy_functor {

public:
    Directory_clone_functor(const std::string& base_dir, const std::string& dst_dir);

    bool make_directory(const std::string& path);
    bool copy_file(const std::string& src, const std::string& dst);
};

//
// class Directory_remove_functor
//

class Directory_remove_functor : public Directory_deep_functor {

    std::string base_dir;

protected:
    bool process_directory(const std::string& filepath);
    bool process_file(const std::string& filepath);
    bool on_access_denied(const std::string& filepath);

public:
    Directory_remove_functor(const std::string& base_dir) : base_dir(base_dir) {}
};

}}

#endif
