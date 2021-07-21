
/*
 *  geo_report.cpp
 *
 *  Created by Christian Lehner on 06/20/2020.
 *  Copyright (c) 2020 Softhub. All rights reserved.
 *
 */

#include "geo_report.h"
#include "geo_config.h"
#include "geo_ip_database.h"
#include "geo_module.h"
#include <hal/hal_strings.h>
#include <net/net.h>

using namespace SOFTHUB::BASE;
using namespace SOFTHUB::HAL;
using namespace SOFTHUB::NET;
using namespace SOFTHUB::UTIL;
using namespace std;

namespace SOFTHUB {
namespace GEOGRAPHY {

//
// class Geo_report
//

Geo_report::Geo_report(ostream& rout) :
    rout(rout),
    option_report_country(false),
    option_report_city(false),
    option_report_state(false),
    option_coords(false),
    option_domain(false),
    option_info(false)
{
}

void Geo_report::output_coords(const Geo_ip_entry* entry)
{
    const Geo_coordinates& coords = entry->get_coordinates();
    float lon = coords.get_longitude().to_degrees<float>();
    float lat = coords.get_latitude().to_degrees<float>();
    rout << lat << " " << lon << endl;
}

void Geo_report::output_location(const Geo_ip_entry* entry, bool all_info)
{
    bool sep = false;
    bool any_info = all_info || option_report_city || option_report_country;
    if (any_info)
        rout << " \"";
    if (all_info || option_report_city) {
        rout << entry->get_city();
        sep = true;
    }
    if (all_info) {
        if (sep)
            rout << ", ";
        rout << entry->get_state();
        sep = true;
    }
    if (all_info || option_report_country) {
        if (sep)
            rout << ", ";
        rout << entry->get_country();
    }
    if (any_info)
        rout << "\"";
    rout << endl;
}

void Geo_report::output_info(const string& ip, const Address* addr, const Geo_ip_entry* entry)
{
    rout << (option_domain ? ip : addr->to_string(false));
    if (option_info)
        output_ns_domain_name(ip);
    bool all_info = !option_report_country && !option_report_city;
    output_location(entry, all_info);
}

void Geo_report::output_ns_domain_name(const string& ip)
{
    stringstream cmd;
    cmd << "nslookup " << ip;
    string output = shell_exec(cmd.str());
    stringstream result(output);
    string line;
    getline(result, line);
    getline(result, line);
    getline(result, line);
    getline(result, line);
    rout << " \"";
    if (line.find("server can't find") != string::npos) {
        output_whois_domain(ip);
    } else {
        getline(result, line);
        output_domain_name_info(line);
    }
    rout << "\"";
}

void Geo_report::output_domain_name_info(const string& info)
{
    String_vector v;
    Strings::split(info, v);
    size_t vlen = v.size();
    if (vlen == 2 && v[0] == "Name:") {
        rout << v[1];
    } else if (vlen >= 4) {
        const string domain = v[3];
        size_t dlen = domain.size();
        if (dlen > 0 && domain[dlen-1] == '.') {
            size_t idx = domain.rfind('.', dlen-2);
            if (idx != string::npos) {
                idx = domain.rfind('.', idx-1);
                if (idx != string::npos) {
                    const string& base = domain.substr(idx+1, dlen-idx-2);
                    rout << base;
                }
            }
        }
    }
}

void Geo_report::output_whois_domain(const string& ip)
{
    stringstream cmd;
    cmd << "whois " << ip;
    string output = shell_exec(cmd.str());
    output_whois_domain_info(output);
}

void Geo_report::output_whois_domain_info(const string& info)
{
    if (output_whois_domain_info_entry(info, "OrgName:"))
        return;
    if (output_whois_domain_info_entry(info, "org-name:"))
        return;
    if (output_whois_domain_info_entry(info, "person:"))
        return;
    if (output_whois_domain_info_entry(info, "netname:"))
        return;
    if (output_whois_domain_info_entry(info, "role:"))
        return;
}

bool Geo_report::output_whois_domain_info_entry(const string& info, const string& match)
{
    stringstream stream(info);
    while (stream.good()) {
        string line;
        getline(stream, line);
        if (String_util::find_case_insensitive(line, match) != string::npos) {
            output_whois_domain_info_value(line);
            return true;
        }
    }
    return false;
}

void Geo_report::output_whois_domain_info_value(const string& line)
{
    String_vector v;
    Strings::split(line, v);
    if (v.size() >= 2) {
        size_t idx = line.find(v[1]);
        rout << line.substr(idx);
    }
}

//
// class Geo_location_report
//

Geo_location_report::Geo_location_report(ostream& rout) :
    Geo_report(rout)
{
}

void Geo_location_report::report_ip(const string& ip)
{
    Geo_ip_database* db = Geo_module::module.instance->get_ip_database();
    Address_ref addr = Address::create_from_dns_name(ip, 0);
    Geo_ip_entry_ref entry = addr ? db->find(addr) : nullptr;
    if (entry) {
        if (option_coords) {
            output_coords(entry);
        } else {
            output_info(ip, addr, entry);
        }
    } else {
        rout << ip << " not found" << endl;
    }
}

//
// class Geo_route_report
//

Geo_route_report::Geo_route_report(ostream& rout) :
    Geo_report(rout), option_eliminate_duplicates(false)
{
}

void Geo_route_report::report_hop(const string& ip, Geo_coordinates* last_pos)
{
    Geo_ip_database* db = Geo_module::module.instance->get_ip_database();
    Address_ref addr = Address::create_from_dns_name(ip, 0);
    Geo_ip_entry_ref entry = addr ? db->find(addr) : nullptr;
    if (entry) {
        const Geo_coordinates& pos = entry->get_coordinates();
        if (option_eliminate_duplicates && last_pos) {
            if (*last_pos == pos)
                return;
            *last_pos = pos;
        }
        if (option_coords) {
            output_coords(entry);
        } else {
            output_info(ip, addr, entry);
        }
    }
}

void Geo_route_report::report_ip(const string& ip)
{
    stringstream cmd;
    cmd << "traceroute -n -w 3 -q 1 -N 32 -m 16 " << ip;
    string route = shell_exec(cmd.str());
    Geo_coordinates last_pos;
    stringstream result(route);
    string line;
    // skip headline
    getline(result, line);
    while (result.good()) {
        getline(result, line);
        String_vector v;
        Strings::split(line, v);
        if (v.size() >= 4)
            report_hop(v[1], &last_pos);
    }
}

//
// class Geo_report_factory
//

Geo_report_factory::Geo_report_factory()
{
}

Geo_report* Geo_report_factory::create_report(const Geo_config* config)
{
    Geo_report* report;
    bool minus_g = config->get_bool_parameter("g");
    bool minus_t = config->get_bool_parameter("t");
    bool minus_T = config->get_bool_parameter("T");
    if (minus_t || minus_T) {
        Geo_route_report* r = new Geo_route_report();
        r->option_eliminate_duplicates = minus_t;
        report = r;
    } else {
        report = new Geo_location_report();
    }
    report->option_report_country = config->get_bool_parameter("C");
    report->option_report_city = config->get_bool_parameter("c");
    report->option_report_state = config->get_bool_parameter("s");
    report->option_domain = config->get_bool_parameter("d");
    report->option_info = config->get_bool_parameter("i");
    report->option_coords = minus_g;
    return report;
}

const string Geo_report_factory::all_options = "cCgditT";

}}
