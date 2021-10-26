
//
//  geo_ip_database.cpp
//
//  Created by Christian Lehner on 21/10/15.
//  Copyright (c) 2019 Softhub. All rights reserved.
//

#include "geo_ip_database.h"
#include "geo_ip_serialization.h"
#include "geo_module.h"
#include <hal/hal.h>
#include <net/net.h>
#include <util/util.h>
#include <fstream>
#if _DEBUG_PERF
#include <sys/time.h>
#endif

using namespace SOFTHUB::BASE;
using namespace SOFTHUB::HAL;
using namespace SOFTHUB::NET;
using namespace SOFTHUB::UTIL;
using namespace std;

namespace SOFTHUB {
namespace GEOGRAPHY {

//
// class Geo_ip_range
//

Geo_ip_range::Geo_ip_range(byte len, unsigned a, unsigned b) : len(len)
{
    if (a <= b) {
        this->lo = a;
        this->hi = b;
    } else {
        this->lo = b;
        this->hi = a;
    }
}

bool Geo_ip_range::in_range(unsigned ip) const
{
    return lo <= ip && ip <= hi;
}

void Geo_ip_range::serialize(BASE::Serializer* serializer) const
{
    serializer->write(len);
    serializer->write(lo);
    serializer->write(hi);
}

void Geo_ip_range::deserialize(BASE::Deserializer* deserializer)
{
    deserializer->read(len);
    deserializer->read(lo);
    deserializer->read(hi);
}

bool Geo_ip_range::operator<(const Geo_ip_range& obj) const
{
    if (len < obj.len)
        return true;
    return lo < obj.lo && hi < obj.hi;
}

bool Geo_ip_range::operator==(const Geo_ip_range& obj) const
{
    return len == obj.len && lo == obj.lo && hi == obj.hi;
}

void Geo_ip_range::stringify(std::ostream& stream, unsigned ip4)
{
    stream <<
        ((ip4 & 0xff000000) >> 24) << "." <<
        ((ip4 & 0x00ff0000) >> 16) << "." <<
        ((ip4 & 0x0000ff00) >> 8) << "." <<
        ((ip4 & 0x000000ff));
}

string Geo_ip_range::to_string() const
{
    stringstream stream;
    stringify(stream, lo);
    stream << "-";
    stringify(stream, hi);
    return stream.str();
}

size_t Geo_ip_range::hash() const
{
    return len * lo % hi;
}

//
// class Geo_ip_entry
//

Geo_ip_entry::Geo_ip_entry(const Geo_ip_range& range, const std::string& country_code, const std::string& country, const std::string& state, const std::string& city, const string& zip, const string& tz, const Geo_coordinates& coordinates) :
    range(range), country_code(country_code), country(country), state(state), city(city), zip(zip), tz(tz), coordinates(coordinates)
{
}

bool Geo_ip_entry::check_valid(const string& s)
{
    return !s.empty() && s != "-";
}

bool Geo_ip_entry::is_valid() const
{
    return check_valid(country) && check_valid(state) && check_valid(city);
}

void Geo_ip_entry::serialize(BASE::Serializer* serializer) const
{
    range.serialize(serializer);
    serializer->write(country_code);
    serializer->write(country);
    serializer->write(state);
    serializer->write(city);
    serializer->write(zip);
    serializer->write(tz);
    coordinates.serialize(serializer);
}

void Geo_ip_entry::deserialize(BASE::Deserializer* deserializer)
{
    range.deserialize(deserializer);
    deserializer->read(country_code);
    deserializer->read(country);
    deserializer->read(state);
    deserializer->read(city);
    deserializer->read(zip);
    deserializer->read(tz);
    coordinates.deserialize(deserializer);
}

bool Geo_ip_entry::operator<(const Interface& obj) const
{
    const Geo_ip_entry* other = dynamic_cast<const Geo_ip_entry*>(&obj);
    assert(other);
    return other && range < other->range;
}

bool Geo_ip_entry::operator==(const Interface& obj) const
{
    const Geo_ip_entry* other = dynamic_cast<const Geo_ip_entry*>(&obj);
    assert(other);
    return this == other || (other && range == other->range);
}

string Geo_ip_entry::to_string() const
{
    stringstream stream;
    stream << city << ", " << state << ", " << country;
    return stream.str();
}

size_t Geo_ip_entry::hash() const
{
    return range.hash() + std::hash<string>()(country_code) + std::hash<string>()(state) % std::hash<string>()(city) * coordinates.hash();
}

//
// class Geo_ip_database
//

void Geo_ip_database::configure(IConfig* config)
{
}

string Geo_ip_database::map_language(const Geo_ip_entry* entry) const
{
    const string& country = entry->get_country();
    const string& city = entry->get_city();
    string language = country;
    if (city.empty())
        language = language_map.get(country);
    else
        language = language_map.get(country + "-" + city);
    if (language.empty()) {
        language = country;
        String_util::to_lower(language);
    }
    return language;
}

void Geo_ip_database::trim(string& s)
{
    String_util::trim(s);
//  String_util::trim(s, ispunct);  // TODO: double check that, may cause issue only on few entries
}

string Geo_ip_database::geo_data_dir()
{
    const IConfig* config = Base_module::get_configuration();
    string data_dir = config->get_parameter("geo-db", DEFAULT_DATA);
    return File_path::concat(data_dir, "geo-db");
}

void Geo_ip_database::define_language(const string& country, const string& state, const string& language)
{
    string key = country;
    if (state.length() > 0)
        key += "-" + state;
    language_map.insert(key, language);
}

String_map Geo_ip_database::language_map;

//
// class Geo_ip_mem_database
//

void Geo_ip_mem_database::configure(IConfig* config)
{
    Geo_ip_database::configure(config);
}

int Geo_ip_mem_database::import(const string& filename)
{
    ifstream stream(filename);
    if (!stream.good())
        return -1;
    data->remove_all();
    string line, ip_from, ip_to, country_code, country, state, city, lat, lon, zip, tz;
    while (stream.good()) {
        getline(stream, line);
        stringstream sstream(line);
        next_column(sstream, ip_from);
        next_column(sstream, ip_to);
        next_column(sstream, country_code);
        next_column(sstream, country);
        next_column(sstream, state);
        next_column(sstream, city);
        next_column(sstream, lat);
        next_column(sstream, lon);
        next_column(sstream, zip);
        next_column(sstream, tz);
        unsigned lo = Address_ip4::aton(ip_from.c_str());
        unsigned hi = Address_ip4::aton(ip_to.c_str());
        const Geo_latitude& a = Geo_latitude::parse(lat);
        const Geo_longitude& b = Geo_longitude::parse(lon);
        Geo_coordinates coords(a, b);
        Geo_ip_range range(4, lo, hi);
        Geo_ip_entry* entry = new Geo_ip_entry(range, country_code, country, state, city, zip, tz, coords);
        data->append(entry);
    }
    stream.close();
    return 0;
}

int Geo_ip_mem_database::rebuild()
{
    // TODO: this is old code and should be removed
    const string& data_dir = geo_data_dir();
    if (!File_path::ensure_dir(data_dir))
        return -5;
    try {
        Geo_ip_serializer serializer(data_dir);
        data->serialize(&serializer);
    } catch (Exception& ex) {
        clog << "rebuild failed: " << ex.get_message() << endl;
        return -6;
    }
    return 0;
}

Geo_ip_entry_ref Geo_ip_mem_database::find(const Address* address) const
{
#if _DEBUG_PERF >= 8
    timing.begin();
#endif
    byte len = (byte) address->get_length();
    unsigned ip = ntohl(address->get_addr_in().sin_addr.s_addr);
    Geo_ip_range range(len, ip, ip);
    Geo_ip_entry_ref entry(new Geo_ip_entry(range));
    bool found = data->binary_search(entry);
#if _DEBUG_PERF >= 8
    long msec = timing.end();
    cdbg << "found address " << address->to_string(false) << " in " << msec << "ms" << endl;
#endif
    return found ? entry : nullptr;
}

void Geo_ip_mem_database::serialize(BASE::Serializer* serializer) const
{
    serializer->write(data);
}

void Geo_ip_mem_database::deserialize(BASE::Deserializer* deserializer)
{
    deserializer->read(data);
}

void Geo_ip_mem_database::next_column(istream& stream, string& s)
{
    string tmp;
    getline(stream, tmp, '"');
    getline(stream, s, '"');
    getline(stream, tmp, ',');
    trim(s);
}

//
// class Geo_ip_file_database
//

void Geo_ip_file_database::configure(IConfig* config)
{
    Geo_ip_database::configure(config);
    const string& data_dir = geo_data_dir();
    clog << "data " << data_dir << endl;
    if (!File_path::exists(data_dir))
        Geo_module::module.instance->recover_state();
}

unsigned Geo_ip_file_database::ip_num_from_string_vector(const String_vector& tokens)
{
    unsigned value = 0;
    for (int i = 0, n = (int) tokens.size(); i < n; i++) {
        int t = atoi(tokens[n-i-1].c_str());
        value |= t << (i * 8);
    }
    return value;
}

Geo_ip_entry_ref Geo_ip_file_database::find_entry(const string& path, Geo_ip_num ip_num) const
{
    Directory dir(path);
    Geo_directory_functor functor(ip_num);
    bool keep_on = dir.for_each_file(functor);
    if (keep_on)
        return 0;
    Geo_ip_entry* entry = functor.get_ip_entry();
    return entry && entry->is_valid() ? entry : 0;
}

#if GEO_IP_DATASET_RANGE_FILES

Geo_ip_entry_ref Geo_ip_file_database::find_entry_dict(const string& path, Geo_ip_num ip_num) const
{
    const string& filename = path + "/ranges";
    FILE* file = fopen(filename.c_str(), "r");
    if (!file)
        return 0;
    bool found = false;
    Geo_ip_entry_ref entry(new Geo_ip_entry());
    Stream_deserializer deserializer(file);
    try {
        while (!feof(file)) {
            entry->deserialize(&deserializer);
            const Geo_ip_range& range = entry->get_range();
            if (range.in_range(ip_num)) {
                found = true;
                break;
            }
        }
    } catch (...) {
        clog << "failed to find range" << endl;
    }
    fclose(file);
    return found ? entry : nullptr;
}

#endif

Geo_ip_entry_ref Geo_ip_file_database::find_in_filesystem_recursively(const string& path, const String_vector& tokens, int idx) const
{
    const string& t = tokens[idx];
    const string& sub_dir = File_path::concat(path, t);
    Geo_ip_entry_ref entry = 0;
    if (File_path::exists(sub_dir)) {
        entry = find_in_filesystem_recursively(sub_dir, tokens, idx+1);
        if (entry)
            return entry;
    }
    Geo_ip_num ip_num = ip_num_from_string_vector(tokens);
#if GEO_IP_DATASET_RANGE_FILES
    return find_entry_dict(path, ip_num);
#else
    return find_entry(path, ip_num);
#endif
}

Geo_ip_entry_ref Geo_ip_file_database::find_in_filesystem(const Address* address) const
{
    Lock::Block lock(mutex);
    string dir = geo_data_dir();
    if (!File_path::exists(dir))
        return 0;
    const string& addr_str = address->to_string(false);
    String_vector tokens;
    String_util::split(addr_str, tokens, ".");
    return find_in_filesystem_recursively(dir, tokens, 0);
}

Geo_ip_entry_ref Geo_ip_file_database::find(const Address* address) const
{
    return find_in_filesystem(address);
}

}}
