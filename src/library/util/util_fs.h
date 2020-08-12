
//
//  util_fs.h
//
//  Created by Christian Lehner on 28/03/17.
//  Copyright (c) 2019 Softhub. All rights reserved.
//

#ifndef SOFTHUB_LIB_UTIL_FS_H
#define SOFTHUB_LIB_UTIL_FS_H

#include "util_log.h"
#include "util_string.h"
#include <base/base.h>
#include <hal/hal.h>
#include <net/net.h>
#include <fstream>

namespace SOFTHUB {
namespace UTIL {

//
// interface File_storeable
//

class File_storeable : public BASE::Serializable {

public:
    virtual std::string get_external_name() const = 0;
};

//
// class File_storage
//

class File_storage : public BASE::Object<> {

    static const int subdir_prefix_exp = 3;
    static const int subdir_prefix_hash;
    static const int max_filename_len = 255;
    static const int long_filename_hash_pow = 5;
    static const int long_filename_hash;

protected:
    std::string base_dir;    // the path to the root of the file system

    std::string make_path(const std::string& sub_dir, const std::string& filename) const;
    std::string ensure_path(const std::string& sub_dir, const std::string& filename) const;
    std::string ensure_path(const std::string& sub_dir, const NET::Url* url) const;
    bool erase_data(const std::string& dir, const std::string& filename) const;
    bool erase_all_data(const std::string& subdir) const;

    static std::string modified_base64_encode(const std::string& s);
    static std::string modified_base64_decode(const std::string& s);

public:
    static const char* default_base_dir;

    File_storage(const std::string& base_dir = default_base_dir);

    const std::string& get_base_dir() const { return base_dir; }
    template <typename T, typename S> void write(const T* obj);
    template <typename T, typename D> void read(T*& obj) const;
    template <typename T, typename D> BASE::Handle<T> find(const std::string& filename, const std::string& dir) const;
    bool erase_all_data();
    std::string make_path(const std::string& sub_dir, const NET::Url* url) const;
};

template <typename T, typename S>
void File_storage::write(const T* obj)
{
    assert(obj);
    const std::string& dir = "geo/";
    const std::string& filename = obj->get_external_name();
    std::string filepath = ensure_path(dir, filename);
    FILE* file = ::fopen(filepath.c_str(), "w");
    if (!file)
        throw new BASE::Serialization_exception(filepath, obj->get_class_id());
    S serializer(file);
    serializer.write(obj);
    ::fclose(file);
}

template <typename T, typename D>
void File_storage::read(T*& obj) const
{
    assert(obj);
    const std::string& dir = "geo/";
    const std::string& filename = obj->get_external_name();
    std::string filepath = ensure_path(dir, filename);
    FILE* file = ::fopen(filepath.c_str(), "r");
    if (!file)
        throw new BASE::Serialization_exception(filepath, obj->get_class_id());
    D deserializer(file);
    deserializer.read((BASE::Serializable*&) obj, obj->get_class_id());
    ::fclose(file);
}

template <typename T, typename D>
BASE::Handle<T> File_storage::find(const std::string& filename, const std::string& dir) const
{
    const std::string& filepath = make_path(dir, filename);
    FILE* file = ::fopen(filepath.c_str(), "r");
    if (!file)
        return BASE::Handle<T>::null;
    BASE::Handle<T> data;
    try {
        D deserializer(file);
        deserializer.read(data);
    } catch (BASE::Serialization_exception& ex) {
        cdbg << "restoring obj '" << filepath << "' failed: " << ex.get_message() << endl;
        HAL::File_path::rename_file(filepath, "." + filepath);
    }
    ::fclose(file);
    return data;
}

//
// class File_storage_serializer
//

class File_storage_serializer : public BASE::Stream_serializer {

    File_storage* storage;

public:
    File_storage_serializer(FILE* file);

    void write(const BASE::Serializable* obj, class_id_type cid);

    using BASE::Stream_serializer::write;
};

//
// class File_storage_deserializer
//

class File_storage_deserializer : public BASE::Stream_deserializer {

    File_storage* storage;

public:
    File_storage_deserializer(FILE* file);

    void read(BASE::Serializable*& obj, class_id_type cid);

    using BASE::Stream_deserializer::read;
};

}}

#endif
