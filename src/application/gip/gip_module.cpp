
//
//  gip_module.cpp
//
//  Created by Christian Lehner on 1/19/20.
//  Copyright (c) 2020 Softhub. All rights reserved.
//

#include "gip_module.h"
#include <geography/geo_module.h>

namespace SOFTHUB {
namespace GIP {

using namespace SOFTHUB::BASE;
using namespace SOFTHUB::GEOGRAPHY;
using namespace SOFTHUB::HAL;
using namespace SOFTHUB::NET;
using namespace SOFTHUB::UTIL;
using namespace std;

BASE::Module<Gip_module> Gip_module::module;

//
// class Gip_module
//

Gip_module::Gip_module()
{
    Geo_module::module.init();
}

Gip_module::~Gip_module()
{
    Geo_module::module.dispose();
}

void Gip_module::init_logging()
{
    string log_path;
    bool success = File_path::app_log_path(log_path, true);
    if (success)
        Logging::init(log_path);
    else
        cout << "logging unavailable" << std::endl;
}

#ifdef _DEBUG

void Gip_module::test()
{
#if 0
    Geo_ip_database* db = Geo_module::module.instance->get_geo_ip_database();
//  Address* ip = Address::create("91.65.11.27", 0);
//  Address* ip = Address::create("93.207.16.223", 0);
//  Address* ip = Address::create("174.127.90.43", 0);
//  Address* ip = Address::create("87.0.163.161", 0);
    Address* ip = Address::create("24.0.0.24", 0);
    Geo_ip_entry_ref entry = db->find_in_filesystem(ip);
    const string& msg = entry ? entry->to_string() : "not found";
    cout << ip->to_string(false) << " " << msg << std::endl;
#endif
}

#endif

}}
