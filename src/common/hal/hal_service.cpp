
//
//  hal_service.cpp
//
//  Created by Christian Lehner on 21/11/15.
//  Copyright (c) 2019 Softhub. All rights reserved.
//

#include "stdafx.h"
#include "hal_service.h"

namespace SOFTHUB {
namespace HAL {

void IService::run(IService* service)
{
#if FEATURE_HAL_THREAD_POOL
    Hal_module::module.instance->run(service);
#else
    assert(!"FEATURE_HAL_THREAD_POOL needed");
#endif
}

}}
