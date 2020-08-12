
/*
 *  geo_module.h
 *
 *  Created by Christian Lehner on 8/26/10.
 *  Copyright (c) 2019 Softhub. All rights reserved.
 *
 */

#ifndef GEO_GEO_MODULE_H
#define GEO_GEO_MODULE_H

#include <base/base_module.h>
#include "geo_ip_database.h"

namespace SOFTHUB {
namespace GEOGRAPHY {

FORWARD_CLASS(Geo_ip_server);
FORWARD_CLASS(Geo_access_server);

class Geo_module : public BASE::Object<> {

    BASE::IConfig_ref config;
    Geo_ip_server_ref ip_server;
    bool server_done;

public:
    Geo_module();
    ~Geo_module();

    Geo_ip_server* get_ip_server() { return ip_server; }
    Geo_ip_database* get_ip_database();
    void configure(BASE::IConfig* config);
    void run_service();
    void terminate_service();
    void save_state(BASE::Serializer* serializer);
    void restore_state(BASE::Deserializer* deserializer);
    void recover_state();

    static BASE::Module<Geo_module> module;
#ifdef _DEBUG
    static void test();
#endif
};

}}

#endif
