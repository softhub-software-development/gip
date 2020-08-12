
//
//  hal.h
//
//  Created by Christian Lehner on 11/18/15.
//  Copyright (c) 2019 Softhub. All rights reserved.
//

#ifndef SOFTHUB_HAL_HAL_H
#define SOFTHUB_HAL_HAL_H

#ifndef FEATURE_HAL_USB_DISCOVERY
#define FEATURE_HAL_USB_DISCOVERY 0
#endif
#ifndef FEATURE_HAL_THREAD_POOL
#define FEATURE_HAL_THREAD_POOL 1
#endif
#ifndef FEATURE_HAL_FS
#define FEATURE_HAL_FS 1
#endif
#ifndef FEATURE_HAL_UTILS
#define FEATURE_HAL_UTILS 1
#endif

#include "hal_condition.h"
#include "hal_directory.h"
#include "hal_filesystem.h"
#include "hal_lock.h"
#include "hal_module.h"
#include "hal_observer.h"
#include "hal_pathname.h"
#include "hal_semaphore.h"
#include "hal_service.h"
#include "hal_strings.h"
#include "hal_thread.h"
#include "hal_time.h"
#if FEATURE_HAL_THREAD_POOL
#include "hal_thread_pool.h"
#endif
#if FEATURE_HAL_USB_DISCOVERY
#include "hal_usb.h"
#endif
#if FEATURE_HAL_UTILS
#include "hal_utils.h"
#endif

#endif
