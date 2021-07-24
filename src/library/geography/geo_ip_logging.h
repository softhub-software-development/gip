
//
//  geo_ip_logging.h
//
//  Created by Christian Lehner on 07/24/21.
//  Copyright (c) 2019 Softhub. All rights reserved.
//

#ifndef SOFTHUB_LIB_GEOGRAPHY_LOGGING_H
#define SOFTHUB_LIB_GEOGRAPHY_LOGGING_H

#include "geo_ip_database.h"
#include <net/net.h>
#include <util/util.h>

namespace SOFTHUB {
namespace GEOGRAPHY {

FORWARD_CLASS(Geo_ip_server);
FORWARD_CLASS(Geo_log_data);
FORWARD_CLASS(Geo_access_log_data);
FORWARD_CLASS(Geo_auth_log_data);
FORWARD_CLASS(Geo_log_listener);
FORWARD_CLASS(Geo_log_consumer);

typedef BASE::Hash_map<NET::Address_const_ref,Geo_log_data_ref> Geo_locations;

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
    virtual void classify(const Geo_ip_server* server) = 0;
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
    void classify(const Geo_ip_server* server);
};

//
// class Geo_auth_log_data
//

class Geo_auth_log_data : public Geo_log_data {

public:
    Geo_auth_log_data(const NET::Address* address, const Geo_ip_entry* ip_entry);

    std::string get_img() const;
    void classify(const Geo_ip_server* server);
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

    Geo_ip_server* get_server() { return server; }
    void run();
    void fail(const std::exception& ex);
    void stop();
    void add_location(const NET::Address* ip, Geo_log_data* data);
    const Geo_locations& get_locations() const { return locations; }
    void clear_locations();

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

    bool process_element(std::istream& stream, std::string& str);
    bool process_bracketed(std::istream& stream, std::string& str);
    bool process_quoted(std::istream& stream, std::string& str);

protected:
    Geo_log_listener_weak_ref listener;

    Geo_log_consumer(Geo_log_listener* listener);

    bool store_column(const std::string& ip, const BASE::String_vector& columns);

    virtual bool consumer_process(const std::string& line);
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
    bool consumer_process(const std::string& line);
    bool consumer_process(const BASE::String_vector& cols);

public:
    Geo_auth_log_consumer(Geo_log_listener* listener);
};

}}

#endif
