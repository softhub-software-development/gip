
/*
 *  util_config.cpp
 *
 *  Created by Christian Lehner on 23/02/20.
 *  Copyright 2020 softhub. All rights reserved.
 */

#include "stdafx.h"
#include "util_config.h"
#include "util_string.h"
#include "util_log.h"

using namespace SOFTHUB::BASE;
using namespace SOFTHUB::HAL;
using namespace SOFTHUB::UTIL;
using namespace std;

namespace SOFTHUB {
namespace UTIL {

//
// class File_config
//

File_config::File_config(const string& name, const string& ext) :
    config_name(name), config_ext(ext)
{
}

void File_config::define_app_data_dir(const string& sub_dir)
{
    app_data_dir = sub_dir;
}

void File_config::read_parameters()
{
    string path;
    if (!File_path::app_data_path(path))
        return;
    ifstream fs;
    const string& file_path = make_path(path, config_name, config_ext);
    fs.open(file_path, fstream::in);
    if (fs.fail()) {
        clog << "config not found: " << file_path << endl;
        return;
    }
    clog << "reading config from '" << file_path << "' ... ";
    clog.flush();
    int lineno = 0;
    string line;
    while (getline(fs, line)) {
        lineno++;
        if (line.find("#") == 0)
            continue;
        String_vector sv;
        String_util::split(line, sv, "= ,;\t");
        size_t svlen = sv.size();
        if (svlen >= 1) {
            string key = sv[0];
            String_util::trim(key);
            String_util::to_lower(key);
            string val = "";
            if (svlen >= 2) {
                val = sv[1];
                String_util::trim(val);
            }
            apply_parameter(key, val);
        } else if (svlen > 0) {
            clog << "error in config file line " << lineno << endl;
        }
    }
    clog << "done reading config file" << endl;
    fs.close();
}

void File_config::apply_parameter(const string& key, const string& val)
{
    if (key == "test-mode") {
        bool state = val == "true" || atoi(val.c_str());
        params[key] = new Config_bool_param(state);
    } else if (key == "debug") {
        bool state = val == "true" || atoi(val.c_str());
        params[key] = new Config_bool_param(state);
    } else {
        params[key] = new Config_string_param(val);
    }
}

string File_config::make_path(const string& path, const string& name, const string& ext)
{
    const string& sub_dir = app_data_dir.empty() ? "" : app_data_dir + Pathname::separator;
    return path + Pathname::separator + sub_dir + name + ext;
}

string File_config::app_data_dir;

}}
