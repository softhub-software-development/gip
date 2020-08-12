
//
//  base_options.cpp
//
//  Created by Christian Lehner on 05/09/15.
//  Copyright (c) 2019 Softhub. All rights reserved.
//

#include "stdafx.h"
#include "base_options.h"
#include <iostream>
#include <fstream>
#include <sstream>
#ifndef PLATFORM_WIN
#include <unistd.h>
#endif

using namespace std;

namespace SOFTHUB {
namespace BASE {

//
// class Config_bool_param
//

string Config_bool_param::get_string_value() const
{
    stringstream stream;
    stream << boolalpha << value;
    return stream.str();
}

//
// class Config_int_param
//

string Config_int_param::get_string_value() const
{
    stringstream stream;
    stream << value;
    return stream.str();
}

//
// class Config_list_param
//

string Config_list_param::get_string_value() const
{
    stringstream stream;
    Config_list::const_iterator it = params.begin();
    Config_list::const_iterator tail = params.end();
    if (it != tail) {
        const Config_param* param = *it++;
        stream << param->get_string_value();
        while (it != tail) {
            param = *it++;
            stream << ", " << param->get_string_value();
        }
    }
    return stream.str();
}

//
// class Config_dict_param
//

string Config_dict_param::get_string_value() const
{
    stringstream stream;
    Config_params::const_iterator it = params.begin();
    Config_params::const_iterator tail = params.end();
    while (it != tail) {
        const Config_params::value_type& pair = *it++;
        const string& key = pair.first;
        const Config_param* val = pair.second;
        const string& type = val->get_type();
        const string& sval = val->get_string_value();
        stream << category << "." << key << "<" << type << "> = " << sval << endl;
    }
    return stream.str();
}

//
// class Configuration
//

#ifdef PLATFORM_WIN

bool Configuration::parse(int argc, char** argv, const char* options)
{
    // TODO
    return false;
}

#else

bool Configuration::parse(int argc, char** argv, const char* options)
{
    if (argc > 0)
        append("programm-name", argv[0]);
    int c;
    while ((c = getopt(argc, argv, options)) >= 0) {
        switch (c) {
        case ':':
            break;
        case '?':
            break;
        default:
            int idx = optarg ? optind - 2 : optind - 1;
            string name = argv[idx] + 1;
            if (optarg) {
                append(name, optarg);
            } else {
                append(name, true);
            }
            break;
        }
    }
    optind = 1;
    return true;
}

#endif

void Configuration::append(const string& key, bool param)
{
    append(key, new Config_bool_param(param));
}

void Configuration::append(const string& key, int param)
{
    append(key, new Config_int_param(param));
}

void Configuration::append(const string& key, const char* param)
{
    const string& s(param ? param : "");
    append(key, new Config_string_param(s));
}

void Configuration::append(const string& key, const string& param)
{
    append(key, new Config_string_param(param));
}

void Configuration::append(const string& key, Config_param* param)
{
    params[key] = param;
}

void Configuration::remove(const string& key)
{
    params.remove(key);
}

void Configuration::apply(const string& key, bool& value) const
{
    const Config_param* param = params.get(key);
    if (param) {
        const Config_int_param* int_param = dynamic_cast<const Config_int_param*>(param);
        if (int_param)
            value = int_param->get_value() != 0;
    }
}

void Configuration::apply(const string& key, int& value) const
{
    const Config_param* param = params.get(key);
    if (param) {
        const Config_int_param* int_param = dynamic_cast<const Config_int_param*>(param);
        if (int_param)
            value = int_param->get_value();
    }
}

void Configuration::apply(const string& key, string& value) const
{
    const Config_param* param = params.get(key);
    if (param) {
        const Config_string_param* str_param = dynamic_cast<const Config_string_param*>(param);
        if (str_param)
            value = str_param->get_value();
    }
}

void Configuration::set_bool_parameter(const string& key, bool value)
{
    set_parameter(key, new Config_bool_param(value));
}

int Configuration::get_bool_parameter(const string& key, bool default_value) const
{
    const Config_param* param = params.get(key);
    const Config_bool_param* bool_param = dynamic_cast<const Config_bool_param*>(param);
    return bool_param ? bool_param->get_value() : default_value;
}

void Configuration::set_parameter(const string& key, int value)
{
    set_parameter(key, new Config_int_param(value));
}

int Configuration::get_parameter(const string& key, int default_value) const
{
    const Config_param* param = params.get(key);
    const Config_int_param* int_param = dynamic_cast<const Config_int_param*>(param);
    return int_param ? int_param->get_value() : default_value;
}

void Configuration::set_parameter(const string& key, const string& value)
{
    set_parameter(key, new Config_string_param(value));
}

string Configuration::get_parameter(const string& key, const string& default_value) const
{
    const Config_param* param = params.get(key);
    const Config_string_param* string_param = dynamic_cast<const Config_string_param*>(param);
    return string_param ? string_param->get_value() : default_value;
}

void Configuration::set_parameter(const string& key, const char* value)
{
    set_parameter(key, new Config_string_param(value));
}

string Configuration::get_parameter(const string& key, const char* default_value) const
{
    return get_parameter(key, string(default_value));
}

void Configuration::set_parameter(const string& key, Config_param* value)
{
    params.remove(key);
    params.insert(key, value);
}

Config_param_ref Configuration::get_parameter(const string& key, Config_param_ref default_value)
{
    return params.get(key, default_value);
}

string Configuration::get_base_directory() const
{
    return "";
}

string Configuration::get_test_data_directory() const
{
#ifdef PLATFORM_WIN
    return "../../../../data/";
#elif defined PLATFORM_MAC
    return "../../../../../data/";
#else
    return "~/sen/src/data/";
#endif
}

string Configuration::read_file(const string& filename)
{
    string data;
    ifstream stream;
    stream.open(filename, fstream::in);
    if (stream.good()) {
        data = string((istreambuf_iterator<char>(stream)), istreambuf_iterator<char>());
        stream >> data;
        stream.close();
    } else {
        cout << "failed to read from " << filename << endl;
    }
    return data;
}

}}
