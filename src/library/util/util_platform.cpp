
//
//  util_platform.cpp
//
//  Created by Christian Lehner on 4/27/15.
//  Copyright (c) 2019 Softhub. All rights reserved.
//

#include "stdafx.h"
#include "util_platform.h"
#include <iostream>

#ifdef PLATFORM_MAC
#include <mach/vm_statistics.h>
#include <mach/mach_types.h>
#include <mach/mach_init.h>
#include <mach/mach_host.h>
#include <mach/mach.h>
#endif

using namespace std;

namespace SOFTHUB {
namespace UTIL {

large Platform::total_memory_usage()
{
    large used_memory;
#ifdef PLATFORM_MAC
    vm_size_t page_size;
    vm_statistics64_data_t vm_stats;
    mach_port_t mach_port = mach_host_self();
    mach_msg_type_number_t count = sizeof(vm_stats) / sizeof(natural_t);
    int host_page_size_status = host_page_size(mach_port, &page_size);
    int host_statistics_status = host_statistics64(mach_port, HOST_VM_INFO, (host_info64_t) &vm_stats, &count);
    if (KERN_SUCCESS == host_page_size_status && KERN_SUCCESS == host_statistics_status) {
        used_memory = ((int64_t) vm_stats.active_count + (int64_t) vm_stats.inactive_count + (int64_t) vm_stats.wire_count) * (int64_t) page_size;
    } else {
#endif
        cout << "unable to determine memory usage" << endl;
        used_memory = 0;
#ifdef PLATFORM_MAC
    }
#endif
    return used_memory;
}

void Platform::print_total_memory_usage()
{
#ifdef PLATFORM_MAC
    vm_size_t page_size;
    vm_statistics64_data_t vm_stats;
    mach_port_t mach_port = mach_host_self();
    mach_msg_type_number_t count = sizeof(vm_stats) / sizeof(natural_t);
    int host_page_size_status = host_page_size(mach_port, &page_size);
    int host_statistics_status = host_statistics64(mach_port, HOST_VM_INFO, (host_info64_t) &vm_stats, &count);
    if (KERN_SUCCESS == host_page_size_status && KERN_SUCCESS == host_statistics_status) {
        long long free_memory = (int64_t) vm_stats.free_count * (int64_t) page_size;
        long long used_memory = ((int64_t) vm_stats.active_count + (int64_t) vm_stats.inactive_count + (int64_t) vm_stats.wire_count) * (int64_t) page_size;
        long free_memory_in_mb = (free_memory + (1 << 19)) / (1 << 20);
        long used_memory_in_mb = (used_memory + (1 << 19)) / (1 << 20);
        cout << "free memory: " << free_memory << " bytes (" << free_memory_in_mb << "mb)";
        cout << ", used memory: " << used_memory << " bytes (" << used_memory_in_mb << "mb)" << endl;
    } else {
#endif
        cout << "unable to determine memory usage" << endl;
#ifdef PLATFORM_MAC
    }
#endif
}

void Platform::print_process_memory_usage()
{
#ifdef PLATFORM_MAC
    struct task_basic_info t_info;
    mach_msg_type_number_t t_info_count = TASK_BASIC_INFO_COUNT;
    int task_info_status = task_info(mach_task_self(), TASK_BASIC_INFO, (task_info_t) &t_info, &t_info_count);
    if (KERN_SUCCESS == task_info_status) {
        long long resident_memory = (int64_t) t_info.resident_size;
        long long virtual_memory = (int64_t) t_info.virtual_size;
        long resident_memory_in_mb = (resident_memory + (1 << 19)) / (1 << 20);
        long virtual_memory_in_mb = (virtual_memory + (1 << 19)) / (1 << 20);
        cout << "resident size: " << resident_memory << " bytes (" << resident_memory_in_mb << "mb)";
        cout << ", virtual size: " << virtual_memory << " bytes (" << virtual_memory_in_mb << "mb)" << endl;
    } else {
#endif
        cout << "unable to determine memory usage" << endl;
#ifdef PLATFORM_MAC
    }
#endif
}

}}
