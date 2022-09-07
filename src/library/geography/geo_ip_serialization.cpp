
//
//  geo_ip_serialization.cpp
//
//  Created by Christian Lehner on 07/24/21.
//  Copyright (c) 2019 Softhub. All rights reserved.
//

#include "geo_ip_serialization.h"
#include "geo_ip_database.h"
#include "geo_module.h"
#include <hal/hal.h>
#include <net/net.h>
#include <util/util.h>
#include <fstream>

using namespace SOFTHUB::BASE;
using namespace SOFTHUB::HAL;
using namespace SOFTHUB::NET;
using namespace SOFTHUB::UTIL;
using namespace std;

namespace SOFTHUB {
namespace GEOGRAPHY {

//
// class Geo_directory_functor
//

Geo_directory_functor::Geo_directory_functor(Geo_ip_num ip_num) :
    ip_entry(new Geo_ip_entry()), ip_num(ip_num)
{
}

bool Geo_directory_functor::is_interrupted() const
{
    return false;
}

bool Geo_directory_functor::is_valid(struct dirent* entry) const
{
    return entry->d_type == DT_REG;
}

bool Geo_directory_functor::operator()(const Directory* directory, struct dirent* entry)
{
    const string& path = directory->get_path();
    const string& filename = File_path::concat(path, entry->d_name);
    FILE* file = fopen(filename.c_str(), "r");
    if (!file)
        return false;
    bool keep_on = false;
    try {
        Stream_deserializer deserializer(file);
        ip_entry->deserialize(&deserializer);
        const Geo_ip_range& range = ip_entry->get_range();
        keep_on = !range.in_range(ip_num);
    } catch (Exception& ex) {
        clog << "failed to read " << filename << endl;
    }
    fclose(file);
    return keep_on;
}

//
// class Geo_ip_serializer
//

Geo_ip_serializer::Geo_ip_serializer(const string& base_dir) : Geo_directory_serializer(base_dir)
{
}

void Geo_ip_serializer::write(const Serializable* obj, class_id_type cid)
{
    switch (cid) {
    case class_id_array:
        Serializer::write(obj, cid);
        break;
    case Geo_ip_database::class_id:
        Serializer::write(obj, cid);
        break;
    case Geo_ip_entry::class_id:
        write(static_cast<const Geo_ip_entry*>(obj));
        break;
    default:
        Abstract_serializer::write(obj, cid);
        break;
    }
}

void Geo_ip_serializer::write(const Geo_ip_entry* obj)
{
    const Geo_ip_range& range = obj->get_range();
#if GEO_IP_DATASET_RANGE_FILES
    FILE* file = open_path_dict(range);
#else
    FILE* file = open_path(range);
#endif
    if (!file)
        throw Serialization_exception("failed to open " + range.to_string());
    try {
        Stream_serializer serializer(file);
        obj->serialize(&serializer);
    } catch (Serialization_exception& ex) {
        clog << "failed to serialize " << range.to_string() << endl;
        fclose(file);
        throw ex;
    }
    fclose(file);
}

string Geo_ip_serializer::make_path(const Geo_ip_range& range) const
{
    bool create_flag;
    string dir = base_dir;
    unsigned lo = range.get_lower();
    unsigned hi = range.get_upper();
    unsigned e0, e1;
    int ix = 24;
    while (ix > 8 && ((e0 = ((lo >> ix) & 0xff)) == (e1 = ((hi >> ix) & 0xff)))) {
        stringstream stream;
        stream << e0;
        dir = File_path::concat(dir, stream.str());
        File_path::make_directory(dir, create_flag);
        ix -= 8;
    }
    stringstream stream0, stream1;
    e0 = (lo >> ix) & 0xff;
    e1 = (hi >> ix) & 0xff;
    stream0 << e0;
    stream1 << e1;
    ix -= 8;
    while (ix >= 0) {
        e0 = (lo >> ix) & 0xff;
        e1 = (hi >> ix) & 0xff;
        stream0 << "." << e0;
        stream1 << "." << e1;
        ix -= 8;
    }
    const string& s0 = stream0.str();
    const string& s1 = stream1.str();
    const string& filename = s0 + "-" + s1;
    return File_path::concat(dir, filename);
}

FILE* Geo_ip_serializer::open_path(const Geo_ip_range& range) const
{
    const string& path = make_path(range);
    return fopen(path.c_str(), "w");
}

#if GEO_IP_DATASET_RANGE_FILES

FILE* Geo_ip_serializer::open_path_dict(const Geo_ip_range& range) const
{
    bool create_flag;
    string dir = base_dir;
    unsigned lo = range.get_lower();
    unsigned hi = range.get_upper();
    unsigned e0, e1;
    int ix = 24;
    while (ix > 8 && ((e0 = ((lo >> ix) & 0xff)) == (e1 = ((hi >> ix) & 0xff)))) {
        stringstream stream;
        stream << e0;
        dir = File_path::concat(dir, stream.str());
        File_path::make_directory(dir, create_flag);
        ix -= 8;
    }
    const string& path = File_path::concat(dir, "ranges");
    return fopen(path.c_str(), "a");
}

#endif

//
// class Geo_ip_deserializer
//

Geo_ip_deserializer::Geo_ip_deserializer(const string& base_dir) : Geo_directory_deserializer(base_dir)
{
}

void Geo_ip_deserializer::read(Serializable*& obj, class_id_type cid)
{
    switch (cid) {
    case Geo_ip_database::class_id:
        Deserializer::read(obj, cid);
        break;
    case Geo_ip_entry::class_id:
        read((Geo_ip_entry*&) obj);
        break;
    default:
        Abstract_deserializer::read(obj, cid);
        break;
    }
}

void Geo_ip_deserializer::read(Geo_ip_entry*& obj)
{
    if (!obj)
        obj = new Geo_ip_entry();
    obj->deserialize(this);
}

}}
