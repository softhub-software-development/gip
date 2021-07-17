
//
//  geo_ip_server.h
//
//  Created by Christian Lehner on 12/29/19.
//  Copyright (c) 2019 Softhub. All rights reserved.
//

#ifndef SOFTHUB_LIB_GEOGRAPHY_SERVER_H
#define SOFTHUB_LIB_GEOGRAPHY_SERVER_H

#include "geo_ip_database.h"
#include <net/net.h>
#include <util/util.h>

#define GEO_SERVER_URL "https://www.openstreetmap.org/export/embed.html"
#define GEO_IP_SERVER_PORT 10101

namespace SOFTHUB {
namespace GEOGRAPHY {

FORWARD_CLASS(Geo_ip_server);
FORWARD_CLASS(Geo_access_server);
FORWARD_CLASS(Geo_log_data);
FORWARD_CLASS(Geo_log_listener);
FORWARD_CLASS(Geo_log_consumer);

typedef BASE::Hash_map<NET::Address_const_ref,Geo_log_data_ref> Geo_locations;

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
    BASE::IConfig* get_config() { return config; }
    void configure(BASE::IConfig* config);
    bool initialize();
    void finalize();
    void serve_page(const NET::Http_service_request* sreq, NET::Http_service_response* sres);
};

//
// class Geo_log_data
//

class Geo_log_data : public BASE::Object<> {

    NET::Address_const_ref address;
    Geo_ip_entry_const_ref ip_entry;
    int accesses;

public:
    Geo_log_data(const NET::Address* address, const Geo_ip_entry* ip_entry);

    const Geo_ip_entry* get_ip_entry() const { return ip_entry; }
    void increment_accesses() { accesses++; }
    int get_accesses() const { return accesses; }

    virtual std::string get_img() const = 0;
    virtual void classify(BASE::IConfig* config) = 0;
};

//
// class Geo_access_log_data
//

class Geo_access_log_data : public Geo_log_data {

    std::string link;
    std::string referer;
    std::string client;
    bool robot;
    bool download;

    void check_link(const BASE::String_vector& sv);
    void check_client(const BASE::String_vector& sv);

public:
    Geo_access_log_data(const NET::Address* address, const Geo_ip_entry* ip_entry);

    void set_link(const std::string& link) { this->link = link; }
    const std::string& get_link() const { return link; }
    void set_referer(const std::string& referer) { this->referer = referer; }
    const std::string& get_referer() const { return referer; }
    void set_client(const std::string& client) { this->client = client; }
    const std::string& get_client() const { return client; }
    std::string get_img() const;
    void classify(BASE::IConfig* config);
};

//
// class Geo_auth_log_data
//

class Geo_auth_log_data : public Geo_log_data {

public:
    Geo_auth_log_data(const NET::Address* address, const Geo_ip_entry* ip_entry);

    std::string get_img() const;
    void classify(BASE::IConfig* config);
};

//
// class Geo_log_listener
//

class Geo_log_listener : public BASE::Object<HAL::Runnable> {

protected:
    Geo_ip_server_weak_ref server;
    Geo_locations locations;
    bool done;

public:
    Geo_log_listener(Geo_ip_server* server);

    void run();
    void fail(const std::exception& ex);
    void stop();
    void add_location(const NET::Address* ip, Geo_log_data* data);
    const Geo_locations& get_locations() const { return locations; }
    void clear_locations();
    Geo_ip_server* get_server() { return server; }

    virtual void store(const NET::Address* addr, Geo_ip_entry* entry, const BASE::String_vector& columns) = 0;
    virtual UTIL::File_observer* get_observer() = 0;
};

//
// class Geo_access_log_listener
//

class Geo_access_log_listener : public Geo_log_listener {

    UTIL::File_observer_ref observer;

public:
    Geo_access_log_listener(Geo_ip_server* server);

    UTIL::File_observer* get_observer() { return observer; }
    void store(const NET::Address* addr, Geo_ip_entry* entry, const BASE::String_vector& columns);
    void run();
};

//
// class Geo_auth_log_listener
//

class Geo_auth_log_listener : public Geo_log_listener {

    UTIL::File_observer_ref observer;

public:
    Geo_auth_log_listener(Geo_ip_server* server);

    UTIL::File_observer* get_observer() { return observer; }
    void store(const NET::Address* addr, Geo_ip_entry* entry, const BASE::String_vector& columns);
    void run();
};

//
// class Geo_log_consumer
//

class Geo_log_consumer : public BASE::Object<UTIL::IFile_consumer> {

protected:
    Geo_log_listener_weak_ref listener;

    Geo_log_consumer(Geo_log_listener* listener);

    virtual bool consumer_process(const BASE::String_vector& cols) = 0;

    void consumer_reset();
};

//
// class Geo_access_log_consumer
//

class Geo_access_log_consumer : public Geo_log_consumer {

protected:
    bool consumer_process(const BASE::String_vector& cols);

public:
    Geo_access_log_consumer(Geo_log_listener* listener);
};

//
// class Geo_auth_log_consumer
//

class Geo_auth_log_consumer : public Geo_log_consumer {

protected:
    bool consumer_process(const BASE::String_vector& cols);

public:
    Geo_auth_log_consumer(Geo_log_listener* listener);
};

}}

#endif
