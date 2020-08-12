
//
//  gip_module.h
//
//  Created by Christian Lehner on 1/19/20.
//  Copyright (c) 2020 Softhub. All rights reserved.
//

#ifndef SOFTHUB_APPLICATION_GIP_MODULE_H
#define SOFTHUB_APPLICATION_GIP_MODULE_H

#include <geography/geo_ip_database.h>

namespace SOFTHUB {
namespace GIP {

//
// class Gip_module
//

class Gip_module : public BASE::Object<> {

public:
	Gip_module();
	~Gip_module();

	static BASE::Module<Gip_module> module;
 
    static void init_logging();

#ifdef _DEBUG
    static void test();
#endif
};

}}

#endif
