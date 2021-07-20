
/*
 *  geo_report.h
 *
 *  Created by Christian Lehner on 06/20/2020.
 *  Copyright (c) 2020 Softhub. All rights reserved.
 *
 */

#ifndef GEOGRAPHY_GEO_REPORT_H
#define GEOGRAPHY_GEO_REPORT_H

#include <util/util.h>

namespace SOFTHUB {
namespace GEOGRAPHY {

FORWARD_CLASS(Geo_report);
FORWARD_CLASS(Geo_config);
FORWARD_CLASS(Geo_coordinates);
FORWARD_CLASS(Geo_location_report);
FORWARD_CLASS(Geo_route_report);
FORWARD_CLASS(Geo_ip_entry);
FORWARD_CLASS(Geo_report_factory);

//
// class Geo_report
//

class Geo_report : public BASE::Object<> {

    friend class Geo_report_factory;

protected:
    std::ostream& rout;

    Geo_report(std::ostream& rout = std::cout);

    bool option_report_country;
    bool option_report_city;
    bool option_report_state;
    bool option_coords;
    bool option_domain;
    bool option_info;

    void output_ns_domain_name(const std::string& ip);
    void output_domain_name_info(const std::string& info);
    void output_whois_domain(const std::string& ip);
    void output_whois_domain_info(const std::string& info);
    bool output_whois_domain_info_entry(const std::string& info, const std::string& match);
    void output_whois_domain_info_value(const std::string& line);

public:
    virtual void report_ip(const std::string& ip) = 0;
    virtual void output_coords(const Geo_ip_entry* entry);
    virtual void output_location(const Geo_ip_entry* entry, bool all_info = false);
    virtual void output_info(const std::string& ip, const NET::Address* addr, const Geo_ip_entry* entry);
};

//
// class Geo_location_report
//

class Geo_location_report : public Geo_report {

    friend class Geo_report_factory;

protected:
    Geo_location_report(std::ostream& rout = std::cout);

public:
    void report_ip(const std::string& ip);
};

//
// class Geo_route_report
//

class Geo_route_report : public Geo_report {

    friend class Geo_report_factory;

protected:
    Geo_route_report(std::ostream& rout = std::cout);

    bool option_eliminate_duplicates;

    void report_hop(const std::string& ip, Geo_coordinates* last_pos);

public:
    void report_ip(const std::string& ip);
};

//
// class Geo_report_factory
//

class Geo_report_factory {

public:
    Geo_report_factory();

    Geo_report* create_report(const Geo_config* config);

    static const std::string all_options;
};

}}

#endif
