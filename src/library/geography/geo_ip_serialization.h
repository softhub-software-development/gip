
//
//  geo_ip_serialization.h
//
//  Created by Christian Lehner on 07/24/21.
//  Copyright (c) 2019 Softhub. All rights reserved.
//

#ifndef GEOGRAPHY_GEO_IP_SERIALIZATION_H
#define GEOGRAPHY_GEO_IP_SERIALIZATION_H

#include "geo_coordinate.h"
#include "geo_ip.h"
#include <net/net.h>
#include <util/util.h>

#ifdef PLATFORM_MAC
#define DEFAULT_DATA "../.."
#else
#define DEFAULT_DATA "."
#endif

namespace SOFTHUB {
namespace GEOGRAPHY {

class Geo_ip_range;

FORWARD_CLASS(Geo_ip_entry);
FORWARD_CLASS(Geo_ip_database);
DECLARE_ARRAY(Geo_ip_entry_ref, Geo_ip_data);

//
// class Geo_directory_functor
//

class Geo_directory_functor : public HAL::Directory_functor {

    Geo_ip_entry_ref ip_entry;
    Geo_ip_num ip_num;

public:
    Geo_directory_functor(Geo_ip_num ip_num);

    Geo_ip_entry_ref get_ip_entry() { return ip_entry; }
    bool is_interrupted() const;
    bool is_valid(struct dirent* entry) const;
    bool operator()(const HAL::Directory* directory, struct dirent* entry);
};

//
// class Geo_directory_serializer
//
// We are only interested in serializing objects, not simple types.
//

class Geo_directory_serializer : public BASE::Abstract_serializer {

protected:
    std::string base_dir;

public:
    Geo_directory_serializer(const std::string& base_dir) : base_dir(base_dir) {}

    large tell() const { return 0; }
    void seek(large pos) {}
    void write(bool val) {}
    void write(char val) {}
    void write(unsigned char val) {}
    void write(short val) {}
    void write(unsigned short val) {}
    void write(int val) {}
    void write(unsigned int val) {}
    void write(char* val, int len) {}
};

//
// class Geo_directory_deserializer
//
// We are only interested in deserializing objects, not simple types.
//

class Geo_directory_deserializer : public BASE::Abstract_deserializer {

protected:
    std::string base_dir;

public:
    Geo_directory_deserializer(const std::string& base_dir) : base_dir(base_dir) {}

    large tell() const { return 0; }
    void seek(large pos) {}
    void read(bool& val) {}
    void read(char& val) {}
    void read(unsigned char& val) {}
    void read(short& val) {}
    void read(unsigned short& val) {}
    void read(int& val) {}
    void read(unsigned int& val) {}
    void read(char* val, int len) {}
};

//
// class Geo_ip_serializer
//

class Geo_ip_serializer : public Geo_directory_serializer {
    
    std::string make_path(const Geo_ip_range& range) const;

    FILE* open_path(const Geo_ip_range& range) const;
    FILE* open_path_dict(const Geo_ip_range& range) const;

public:
    Geo_ip_serializer(const std::string& base_dir);

    void write(const BASE::Serializable* obj, class_id_type cid);
    void write(const Geo_ip_entry* entry);
};

//
// class Geo_ip_deserializer
//

class Geo_ip_deserializer : public Geo_directory_deserializer {

public:
    Geo_ip_deserializer(const std::string& base_dir);

    void read(BASE::Serializable*& obj, class_id_type cid);
    void read(Geo_ip_entry*& entry);
};

}}

#endif
