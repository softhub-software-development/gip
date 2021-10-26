
//
//  geo_ip_database.h
//
//  Created by Christian Lehner on 21/10/15.
//  Copyright (c) 2019 Softhub. All rights reserved.
//

#ifndef GEOGRAPHY_GEO_IP_DATABASE_H
#define GEOGRAPHY_GEO_IP_DATABASE_H

#include "geo_coordinate.h"
#include "geo_ip.h"
#include <net/net.h>
#include <util/util.h>

#ifdef PLATFORM_MAC
#define DEFAULT_DATA "../.."
#else
#define DEFAULT_DATA "."
#endif

#define GEO_IP_DATASET_RANGE_FILES  0

namespace SOFTHUB {
namespace GEOGRAPHY {

FORWARD_CLASS(Geo_ip_entry);
FORWARD_CLASS(Geo_ip_database);
FORWARD_CLASS(Geo_ip_mem_database);
FORWARD_CLASS(Geo_ip_file_database);
DECLARE_ARRAY(Geo_ip_entry_ref, Geo_ip_data);

typedef unsigned Geo_ip_num;

//
// class Geo_ip_range
//

class Geo_ip_range {

    byte len;
    Geo_ip_num lo;
    Geo_ip_num hi;

    static void stringify(std::ostream& stream, unsigned ip4);

public:
    Geo_ip_range() : len(0), lo(0), hi(0) {}
    Geo_ip_range(byte len, unsigned a, unsigned b);

    byte get_length() const { return len; }
    Geo_ip_num get_lower() const { return lo; }
    Geo_ip_num get_upper() const { return hi; }
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
// class Geo_ip_mem_database
//

class Geo_ip_mem_database : public Geo_ip_database {

    Geo_ip_data_ref data;

    int rebuild();

    static void next_column(std::istream& stream, std::string& s);

public:
    Geo_ip_mem_database() : data(new Geo_ip_data()) {}

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

    Geo_ip_entry_ref find_in_filesystem(const NET::Address* address) const;
    Geo_ip_entry_ref find_in_filesystem_recursively(const std::string& path, const BASE::String_vector& tokens, int idx) const;
    Geo_ip_entry_ref find_entry(const std::string& path, Geo_ip_num ip_num) const;
    Geo_ip_entry_ref find_entry_dict(const std::string& path, Geo_ip_num ip_num) const;

    static Geo_ip_num ip_num_from_string_vector(const BASE::String_vector& tokens);

public:
    Geo_ip_file_database() {}

    void configure(BASE::IConfig* config);
    Geo_ip_entry_ref find(const NET::Address* address) const;
    std::string map_language(const Geo_ip_entry* entry) const;

    DECLARE_CLASS('sifd');
};

}}

#endif
