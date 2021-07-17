
/*
 *  util_config.h
 *
 *  Created by Christian Lehner on 23/02/20.
 *  Copyright 2020 softhub. All rights reserved.
 */

#ifndef LIB_UTIL_CONFIG_H
#define LIB_UTIL_CONFIG_H

#include <base/base.h>
#include <hal/hal.h>

namespace SOFTHUB {
namespace UTIL {

FORWARD_CLASS(File_config);

//
// class File_config
//

class File_config : public BASE::Configuration {

    std::string config_name;
    std::string config_ext;

    virtual void apply_parameter(const std::string& key, const std::string& val);
    
    inline static int is_param_value(int c)
    {
        return isspace(c) || c == '\"' || c == '\'';
    }

public:
    File_config(const std::string& name = "default", const std::string& ext = ".conf");

    void read_parameters();

    static void define_app_data_dir(const std::string& sub_dir);
    static std::string make_path(const std::string& path, const std::string& name, const std::string& ext);

    static std::string app_data_dir;
};

}}

#endif
