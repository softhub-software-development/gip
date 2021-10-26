
/*
 *  geo_module.cpp
 *
 *  Created by Christian Lehner on 8/26/10.
 *  Copyright (c) 2019 Softhub. All rights reserved.
 *
 */

#include "stdafx.h"
#include "geo_module.h"
#include "geo_ip_server.h"
#include "geo_ip_serialization.h"
#include <base/base.h>

#ifndef NO_GEO_PARSER
extern void geo_altitude_test();
extern void geo_coordinate_test();
#endif

namespace SOFTHUB {
namespace GEOGRAPHY {

using namespace BASE;
using namespace HAL;
using namespace NET;
using namespace UTIL;
using namespace std;

BASE::Module<Geo_module> Geo_module::module;

Geo_module::Geo_module() : ip_server(new Geo_ip_server()), server_done(false)
{
    Base_module::module.init();
    Base_module::register_class<Geo_ip_entry>();
    Base_module::register_class<Geo_ip_mem_database>();
    Base_module::register_class<Geo_ip_file_database>();
    Hal_module::module.init();
    Net_module::module.init();
    Util_module::module.init();
#ifdef NO_GEO_PARSER
//  geo_ip_database->import("../../../../data/dbip-country.csv");
    Geo_ip_database::define_language("AT", "", "de");
    Geo_ip_database::define_language("AU", "", "en");
    Geo_ip_database::define_language("CA", "", "en");
    Geo_ip_database::define_language("CA", "Quebec", "fr");
    Geo_ip_database::define_language("CH", "", "de");
    Geo_ip_database::define_language("CH", "Valais", "fr");
    Geo_ip_database::define_language("CH", "Vaud", "fr");
    Geo_ip_database::define_language("CH", "Neuchâtel", "fr");
    Geo_ip_database::define_language("CH", "Geneve", "fr");
    Geo_ip_database::define_language("CH", "Genève", "fr");
    Geo_ip_database::define_language("CH", "Ticino", "it");
    Geo_ip_database::define_language("UK", "", "en");
    Geo_ip_database::define_language("US", "", "en");
#endif
}

Geo_module::~Geo_module()
{
    Util_module::module.dispose();
    Net_module::module.dispose();
    Hal_module::module.dispose();
    Base_module::unregister_class<Geo_ip_entry>();
    Base_module::unregister_class<Geo_ip_mem_database>();
    Base_module::unregister_class<Geo_ip_file_database>();
    Base_module::module.dispose();
}

Geo_ip_database* Geo_module::get_ip_database()
{
    return ip_server->get_ip_database();
}

void Geo_module::configure(IConfig* config)
{
    this->config = config;
    ip_server->configure(config);
}

void Geo_module::run_service()
{
    ip_server->start();
    // TODO: refine termination
    while (!server_done)
        sleep(100);
}

void Geo_module::terminate_service()
{
    server_done = true;
}

void Geo_module::save_state(Serializer* serializer)
{
}

void Geo_module::restore_state(Deserializer* deserializer)
{
}

void Geo_module::recover_state()
{
    Geo_ip_mem_database_ref db(new Geo_ip_mem_database());
    db->configure(config);
    const string& data_dir = db->geo_data_dir();
    const string& base_dir = File_path::basepath_of(data_dir);
    const string& file_name = config->get_parameter("geo-db-csv", "geo-db.csv");
    const string& data_file = File_path::concat(base_dir, file_name);
    cout << "recover state from " << data_file << std::endl;
    if (db->import(data_file)) {
        cout << "import failed" << std::endl;
    } else if (File_path::ensure_dir(data_dir)) {
        Geo_ip_serializer serializer(data_dir);
        db->serialize(&serializer);
    } else {
        cout << "cannot create " << data_dir << std::endl;
    }
}

#ifdef _DEBUG

static void test_coordinates()
{
    const Geo_coordinates& coords = Geo_coordinates::parse("33 3' 3\" N 22 2' 12\" W");
    const string& cstr = coords.to_string(decimal);
    assert(!cstr.empty());
}

void Geo_module::test()
{
    Address_ref ip = Address::create("91.64.54.207", 0);
    const Geo_ip_database* db = Geo_module::module.instance->get_ip_database();
    Geo_ip_entry_ref entry = db->find(ip);
    assert(entry ? entry->get_country() == "DE" : true);
    Address_ref ip2 = Address::create("103.148.139.43", 0); // this ip is missing in dataset
    Geo_ip_entry_ref entry2 = db->find(ip2);
//  assert(entry);
#ifdef NO_GEO_PARSER
    geo_altitude_test();
    geo_coordinate_test();
#endif
    test_coordinates();
}

#endif

}}
