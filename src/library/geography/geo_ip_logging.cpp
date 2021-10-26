
//
//  geo_ip_logging.cpp
//
//  Created by Christian Lehner on 07/24/21.
//  Copyright (c) 2019 Softhub. All rights reserved.
//

#include "stdafx.h"
#include "geo_ip_server.h"
#include "geo_ip_logging.h"
#include "geo_module.h"
#include <iomanip>

using namespace SOFTHUB::BASE;
using namespace SOFTHUB::HAL;
using namespace SOFTHUB::NET;
using namespace SOFTHUB::UTIL;
using namespace std;

namespace SOFTHUB {
namespace GEOGRAPHY {

//
// class Geo_log_data
//

Geo_log_data::Geo_log_data(const Address* address, const Geo_ip_entry* ip_entry) :
    address(address), ip_entry(ip_entry), accesses(0)
{
}

//
// class Geo_access_log_data
//

Geo_access_log_data::Geo_access_log_data(const Address* address, const Geo_ip_entry* ip_entry) :
    Geo_log_data(address, ip_entry), robot(false), download(false)
{
}

void Geo_access_log_data::check_link(const String_vector& sv)
{
    string tmp = link;
    Strings::to_lower(tmp);
    download |= Strings::match_string_ext(tmp, sv);
}

void Geo_access_log_data::check_client(const String_vector& sv)
{
    string tmp = client;
    Strings::to_lower(tmp);
    robot |= Strings::match_string(tmp, sv);
}

string Geo_access_log_data::get_img() const
{
    return robot ? "robot.png" : (download ? "download.png" : "client.png");
}

void Geo_access_log_data::classify(const Geo_ip_server* server)
{
    const String_vector& downloads = server->get_downloads();
    check_link(downloads);
    const String_vector& bots = server->get_bots();
    check_client(bots);
    increment_accesses();
}

//
// class Geo_auth_log_data
//

Geo_auth_log_data::Geo_auth_log_data(const Address* address, const Geo_ip_entry* ip_entry) :
    Geo_log_data(address, ip_entry)
{
}

string Geo_auth_log_data::get_img() const
{
    return "burglar.png";
}

void Geo_auth_log_data::classify(const Geo_ip_server* server)
{
    increment_accesses();
}

//
// class Geo_log_listener
//

Geo_log_listener::Geo_log_listener(Geo_ip_server* server) :
    server(server), done(false)
{
}

void Geo_log_listener::run()
{
    IConfig* config = server->get_config();
    const string& log = config->get_parameter("geo-log", "/var/log/apache2/access.log");
    File_observer* observer = get_observer();
    observer->tail(log, true);
}

void Geo_log_listener::fail(const exception& ex)
{
}

void Geo_log_listener::stop()
{
    done = true;
}

void Geo_log_listener::add_location(const Address* addr, Geo_log_data* data)
{
    locations.insert(addr, data);
}

void Geo_log_listener::clear_locations()
{
    locations.clear();
}

//
// class Geo_access_log_listener
//

Geo_access_log_listener::Geo_access_log_listener(Geo_ip_server* server) :
    Geo_log_listener(server), observer(new File_observer(new Geo_access_log_consumer(this)))
{
}

void Geo_access_log_listener::run()
{
    IConfig* config = server->get_config();
    const string& log = config->get_parameter("geo-access-log", "/var/log/apache2/access.log");
    observer->tail(log, true);
}

void Geo_access_log_listener::store(const Address* addr, Geo_ip_entry* entry, const String_vector& columns)
{
    Geo_locations::const_iterator it = locations.find(addr);
    Geo_access_log_data_ref data;
    if (it == locations.end()) {
        data = new Geo_access_log_data(addr, entry);
        add_location(addr, data);
    } else {
        data = it->second.cast<Geo_access_log_data>();
    }
    size_t ncols = columns.size();
    if (ncols > 4) {
        String_vector sv;
        String_util::split(columns[4], sv);
        if (sv.size() > 1)
            data->set_link(sv[1]);
    }
    if (ncols > 7)
        data->set_referer(columns[7]);
    if (ncols > 8)
        data->set_client(columns[8]);
    data->classify(server);
}

//
// class Geo_auth_log_listener
//

Geo_auth_log_listener::Geo_auth_log_listener(Geo_ip_server* server) :
    Geo_log_listener(server), observer(new File_observer(new Geo_auth_log_consumer(this)))
{
}

void Geo_auth_log_listener::run()
{
    IConfig* config = server->get_config();
    const string& log = config->get_parameter("geo-auth-log", "/var/log/auth.log");
    observer->tail(log, true);
}

void Geo_auth_log_listener::store(const Address* addr, Geo_ip_entry* entry, const String_vector& columns)
{
    Geo_locations::const_iterator it = locations.find(addr);
    Geo_auth_log_data_ref data;
    if (it == locations.end()) {
        data = new Geo_auth_log_data(addr, entry);
        add_location(addr, data);
    } else {
        data = it->second.cast<Geo_auth_log_data>();
    }
    data->classify(server);
}

//
// class Geo_log_consumer
//

Geo_log_consumer::Geo_log_consumer(Geo_log_listener* listener) : listener(listener)
{
}

void Geo_log_consumer::consumer_reset()
{
    const Geo_locations& locations = listener->get_locations();
    clog << "consumer reset: " << locations.size() << endl;
    listener->clear_locations();
}

bool Geo_log_consumer::consumer_process(const string& line)
{
    bool success = true;
    String_vector cols;
    stringstream sstream(line);
    do {
        if (sstream.eof())
            break;
        string str;
        char head = (char) sstream.peek();
        switch (head) {
        default:
            success = process_element(sstream, str);
            break;
        case '[':
            success = process_bracketed(sstream, str);
            break;
        case '"':
            success = process_quoted(sstream, str);
            break;
        }
        String_util::trim(str);
        cols.append(str);
    } while (success);
    consumer_process(cols);
    return true;
}

bool Geo_log_consumer::process_element(istream& stream, string& str)
{
    getline(stream, str, ' ');
    return !str.empty();
}

bool Geo_log_consumer::process_bracketed(istream& stream, string& str)
{
    getline(stream, str, ']');
    if (str.empty())
        return false;
    string tmp;
    getline(stream, tmp, ' ');
    str += ']';
    return true;
}

bool Geo_log_consumer::process_quoted(istream& stream, string& str)
{
    stream.get();
    getline(stream, str, '"');
    if (str.empty())
        return false;
    string tmp;
    getline(stream, tmp, ' ');
    str = '"' + str + '"';
    return true;
}

bool Geo_log_consumer::store_column(const string& ip, const String_vector& columns)
{
    Address_const_ref addr = Address::create_from_dns_name(ip, 0);
    if (!addr)
        return false;
    Geo_ip_server* server = listener->get_server();
    Geo_ip_file_database* database = server->get_ip_database();
    Geo_ip_entry_ref entry = database->find(addr);
    if (!entry)
        return false;
    listener->store(addr, entry, columns);
    return true;
}

//
// class Geo_access_log_consumer
//

Geo_access_log_consumer::Geo_access_log_consumer(Geo_log_listener* listener) : Geo_log_consumer(listener)
{
}

bool Geo_access_log_consumer::consumer_process(const String_vector& columns)
{
    size_t ncols = columns.size();
    if (ncols < 1)
        return false;
    const string& ip = columns[0];
    store_column(ip, columns);
    return true;
}

//
// class Geo_auth_log_consumer
//

Geo_auth_log_consumer::Geo_auth_log_consumer(Geo_log_listener* listener) : Geo_log_consumer(listener)
{
}

bool Geo_auth_log_consumer::consumer_process(const string& line)
{
    return Geo_log_consumer::consumer_process(line);
}

bool Geo_auth_log_consumer::consumer_process(const String_vector& columns)
{
    // report potential break in attempts
    size_t ncols = columns.size();
    if (ncols <= 12)
        return false;
    int ip_idx = -1;
    if (columns[5] == "Failed" && columns[6] == "password")
        ip_idx = 12;
    if (columns[5] == "Did" && columns[6] == "not" && columns[7] == "receive" && columns[8] == "identification")
        ip_idx = 11;
    if (ip_idx < 0)
        return false;
    const string& ip = columns[ip_idx];
    store_column(ip, columns);
    return true;
}

}}
