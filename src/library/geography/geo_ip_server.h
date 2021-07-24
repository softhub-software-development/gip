
//
//  geo_ip_server.h
//
//  Created by Christian Lehner on 12/29/19.
//  Copyright (c) 2019 Softhub. All rights reserved.
//

#ifndef SOFTHUB_LIB_GEOGRAPHY_SERVER_H
#define SOFTHUB_LIB_GEOGRAPHY_SERVER_H

#include "geo_ip_database.h"
#include "geo_ip_logging.h"
#include <net/net.h>
#include <util/util.h>

#define GEO_SERVER_URL "https://www.openstreetmap.org/export/embed.html"
#define GEO_IP_SERVER_PORT 10101

namespace SOFTHUB {
namespace GEOGRAPHY {

FORWARD_CLASS(Geo_ip_server);
FORWARD_CLASS(Geo_log_listener);

//
// class Geo_ip_server
//

class Geo_ip_server : public NET::Http_server {

    static Geo_ip_entry_ref unknown_ip_entry;

    void output_header(std::ostream& stream);
    void output_script(std::ostream& stream, float zoom);
    void output_access_data_element(const Geo_locations::value_type& pair, std::ostream& stream);
    void output_location(const Geo_ip_entry* entry, const NET::Http_service_request* sreq, std::ostream& stream);
    void output_route(const Geo_ip_entry* entry, const NET::Http_service_request* sreq, std::ostream& stream);
    void output_position_data(std::ostream& stream, const Geo_locations& locations);
    void output_access_data(std::ostream& stream);
    void output_auth_data(std::ostream& stream);
    void output_data(std::ostream& stream);

    BASE::String_vector downloads;
    BASE::String_vector bots;

protected:
    HAL::Mutex mutex;
    BASE::IConfig_ref config;
    Geo_ip_file_database_ref database;
    Geo_log_listener_ref access_log_listener;
    Geo_log_listener_ref auth_log_listener;

    void service_control_event();
    void serve_default_page_content(std::ostream& stream);
    void serve_default_page(const NET::Http_service_request* sreq, NET::Http_service_response* sres);
    void serve_echo(const NET::Http_service_request* sreq, NET::Http_service_response* sres);
    void serve_location(const NET::Http_service_request* sreq, NET::Http_service_response* sres);
    void serve_traffic(const NET::Http_service_request* sreq, NET::Http_service_response* sres);
    void serve_data(const NET::Http_service_request* sreq, NET::Http_service_response* sres);
    void serve_content(const std::string& content, const std::string& content_type, NET::Http_service_response* sres);
    void serve_error_page(const std::string& msg, NET::Http_service_response* sres);

public:
    Geo_ip_server();

    Geo_ip_file_database* get_ip_database() { return database; }
    const BASE::String_vector& get_downloads() const { return downloads; }
    const BASE::String_vector& get_bots() const { return bots; }
    BASE::IConfig* get_config() { return config; }
    void configure(BASE::IConfig* config);
    bool initialize();
    void finalize();
    void serve_page(const NET::Http_service_request* sreq, NET::Http_service_response* sres);
};

}}

#endif
