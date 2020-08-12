
//
//  hal_utils.h
//
//  Created by Christian Lehner on 20/04/16.
//  Copyright (c) 2019 Softhub. All rights reserved.
//

#ifndef SOFTHUB_HAL_USB_H
#define SOFTHUB_HAL_USB_H

#include <base/base.h>
#include <iostream>

namespace SOFTHUB {
namespace HAL {

FORWARD_CLASS(USB_device);
DECLARE_ARRAY(USB_device_ref, USB_devices);

typedef BASE::Hash_map<std::string,USB_device_ref> USB_device_map;

//
// class USB_device
//

class USB_device : public BASE::Object<> {

    std::string name;

public:
    USB_device(const std::string& name) : name(name) {}

    const std::string& get_name() const { return name; }
};

//
// class USB_discovery
//

class USB_discovery {

    USB_device_map device_map;
    USB_device_ref null;

    void register_all_devices();
    void unregister_all_devices();

public:
    static USB_discovery* instance;

    static void initialize();
    static void finalize();

    void start();
    void stop();

    USB_device_ref find_device(const std::string& name);
    USB_devices_const_ref find_all_devices();
    void register_device(const std::string& name, USB_device* device);
    void unregister_device(const std::string& name);

    void device_names_of(USB_devices* devices, BASE::String_vector& names);
};

}}

#endif
