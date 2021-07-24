
//
//  geo_ip_database.h
//
//  Created by Christian Lehner on 21/10/15.
//  Copyright (c) 2019 Softhub. All rights reserved.
//

#ifndef GEOGRAPHY_GEO_IP_DATABASE_H
#define GEOGRAPHY_GEO_IP_DATABASE_H

#include "geo_coordinate.h"
#include <net/net.h>
#include <util/util.h>

#ifdef PLATFORM_MAC
#define DEFAULT_DATA "../.."
#else
#define DEFAULT_DATA "."
#endif

namespace SOFTHUB {
namespace GEOGRAPHY {

FORWARD_CLASS(Geo_ip_entry);
FORWARD_CLASS(Geo_ip_database);
FORWARD_CLASS(Geo_ip_ram_database);
FORWARD_CLASS(Geo_ip_file_database);
DECLARE_ARRAY(Geo_ip_entry_ref, Geo_ip_data);

//
// class Geo_ip_range
//

class Geo_ip_range {

    byte len;
    unsigned lo;
    unsigned hi;

    static void stringify(std::ostream& stream, unsigned ip4);

public:
    Geo_ip_range() : len(0), lo(0), hi(0) {}
    Geo_ip_range(byte len, unsigned a, unsigned b);

    byte get_length() const { return len; }
    unsigned get_lower() const { return lo; }
    unsigned get_upper() const { return hi; }
    bool in_range(unsigned value) const;
    void serialize(BASE::Serializer* serializer) const;
    void deserialize(BASE::Deserializer* deserializer);
    bool operator<(const Geo_ip_range& obj) const;
    bool operator==(const Geo_ip_range& obj) const;
    std::string to_string() const;
    size_t hash() const;
};

//
// class Geo_ip_entry
//

class Geo_ip_entry : public BASE::Object<> {

    Geo_ip_range range;
    std::string country_code;
    std::string country;
    std::string state;
    std::string city;
    std::string zip;
    std::string tz;
    Geo_coordinates coordinates;

    static bool check_valid(const std::string& s);

public:
    Geo_ip_entry() {}
    Geo_ip_entry(const Geo_ip_range& range) : range(range) {}
    Geo_ip_entry(const Geo_ip_range& range, const std::string& country_code, const std::string& country, const std::string& state, const std::string& city, const std::string& zip, const std::string& tz, const Geo_coordinates& coordinates);

    const Geo_ip_range& get_range() const { return range; }
    const Geo_coordinates& get_coordinates() const { return coordinates; }
    const std::string& get_country_code() const { return country_code; }
    const std::string& get_country() const { return country; }
    const std::string& get_state() const { return state; }
    const std::string& get_city() const { return city; }
    const std::string& get_zip() const { return zip; }
    const std::string& get_tz() const { return tz; }
    bool is_valid() const;
    void serialize(BASE::Serializer* serializer) const;
    void deserialize(BASE::Deserializer* deserializer);
    bool operator<(const BASE::Interface& obj) const;
    bool operator==(const BASE::Interface& obj) const;
    std::string to_string() const;
    size_t hash() const;

    DECLARE_CLASS('sipe');
};

//
// class Geo_ip_database
//

class Geo_ip_database : public BASE::Object<> {

protected:
    HAL::Mutex mutex;
#if _DEBUG_PERF >= 8
    UTIL::Timing timing;
#endif

    static BASE::String_map language_map;

    static void trim(std::string& s);

public:
    Geo_ip_database() {}

    void configure(BASE::IConfig* config);
    std::string map_language(const Geo_ip_entry* entry) const;
    virtual Geo_ip_entry_ref find(const NET::Address* address) const = 0;

    static void define_language(const std::string& country, const std::string& state, const std::string& language);
    static std::string geo_data_dir();
};

//
// class Geo_ip_ram_database
//

class Geo_ip_ram_database : public Geo_ip_database {

    Geo_ip_data_ref data;

    int rebuild();

    static void next_column(std::istream& stream, std::string& s);

public:
    Geo_ip_ram_database() : data(new Geo_ip_data()) {}

    void configure(BASE::IConfig* config);
    int import(const std::string& filename);
    Geo_ip_entry_ref find(const NET::Address* address) const;
    void serialize(BASE::Serializer* serializer) const;
    void deserialize(BASE::Deserializer* deserializer);

    DECLARE_CLASS('simd');
};

//
// class Geo_ip_file_database
//

class Geo_ip_file_database : public Geo_ip_database {

    Geo_ip_entry_ref find_entry(const std::string& path, unsigned ip_num) const;
    Geo_ip_entry_ref find_in_filesystem_recursively(const std::string& path, const BASE::String_vector& tokens, int idx) const;

    static unsigned ip_num_from_string_vector(const BASE::String_vector& tokens);

public:
    Geo_ip_file_database() {}

    void configure(BASE::IConfig* config);
    Geo_ip_entry_ref find(const NET::Address* address) const;
    Geo_ip_entry_ref find_in_filesystem(const NET::Address* address) const;
    std::string map_language(const Geo_ip_entry* entry) const;

    DECLARE_CLASS('sifd');
};

//
// class Geo_directory_functor
//

class Geo_directory_functor : public HAL::Directory_functor {

    Geo_ip_entry_ref ip_entry;
    unsigned ip_num;

public:
    Geo_directory_functor(unsigned ip_num) : ip_entry(new Geo_ip_entry()), ip_num(ip_num) {}

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

    FILE* open_path(const Geo_ip_range& range);

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
