
//
//  base_options.h
//
//  Created by Christian Lehner on 05/09/15.
//  Copyright (c) 2019 Softhub. All rights reserved.
//

#ifndef BASE_OPTIONS_H
#define BASE_OPTIONS_H

#include "base_stl_wrapper.h"

namespace SOFTHUB {
namespace BASE {

FORWARD_CLASS(IConfig);
FORWARD_CLASS(Configuration);
FORWARD_CLASS(Config_param);
FORWARD_CLASS(Config_int_param);
FORWARD_CLASS(Config_string_param);
FORWARD_CLASS(Config_list_param);
FORWARD_CLASS(Config_dict_param);

typedef Hash_map<std::string,Config_param_ref> Config_params;
typedef List<Config_param_ref> Config_list;

//
// class Config_param
//

class Config_param : public Object<> {

public:
    virtual std::string get_type() const = 0;
    virtual std::string get_string_value() const = 0;
};

//
// class Config_bool_param
//

class Config_bool_param : public Config_param {

    bool value;

public:
    Config_bool_param(bool value) : value(value) {}

    std::string get_type() const { return "bool"; }
    bool get_value() const { return value; }
    std::string get_string_value() const;
};

//
// class Config_int_param
//

class Config_int_param : public Config_param {

    int value;

public:
    Config_int_param(int value) : value(value) {}

    std::string get_type() const { return "int"; }
    int get_value() const { return value; }
    std::string get_string_value() const;
};

//
// class Config_string_param
//

class Config_string_param : public Config_param {

    std::string value;

public:
    Config_string_param(const std::string& value) : value(value) {}

    std::string get_type() const { return "string"; }
    const std::string& get_value() const { return value; }
    std::string get_string_value() const { return value; }
};

//
// class Config_list_param
//

class Config_list_param : public Config_param {

    std::string category;
    Config_list params;

public:
    Config_list_param(const std::string& category) : category(category) {}

    std::string get_type() const { return "list"; }
    void add_list_param(Config_param* param) { params.append(param); }
    const Config_list& get_params() const { return params; }
    Config_list& get_params() { return params; }
    std::string get_string_value() const;
};

//
// class Config_dict_param
//

class Config_dict_param : public Config_param {

    std::string category;
    Config_params params;

public:
    Config_dict_param(const std::string& category) : category(category) {}

    std::string get_type() const { return "dict"; }
    const std::string& get_category() const { return category; }
    void add_dict_param(const std::string& key, Config_param* param) { params.insert(key, param); }
    const Config_params& get_params() const { return params; }
    Config_params& get_params() { return params; }
    std::string get_string_value() const;
};

//
// interface IConfig
//

class IConfig : public Interface {

public:
    virtual bool parse(int argc, char** argv, const char* options) = 0;
    virtual void append(const std::string& key, bool param) = 0;
    virtual void append(const std::string& key, int param) = 0;
    virtual void append(const std::string& key, const char* param) = 0;
    virtual void append(const std::string& key, const std::string& param) = 0;
    virtual void append(const std::string& key, Config_param* param) = 0;
    virtual void remove(const std::string& key) = 0;
    virtual void apply(const std::string& key, bool& value) const = 0;
    virtual void apply(const std::string& key, int& value) const = 0;
    virtual void apply(const std::string& key, std::string& value) const = 0;
    virtual void set_bool_parameter(const std::string& key, bool value) = 0;
    virtual int get_bool_parameter(const std::string& key, bool default_value = false) const = 0;
    virtual void set_parameter(const std::string& key, int value) = 0;
    virtual int get_parameter(const std::string& key, int default_value) const = 0;
    virtual void set_parameter(const std::string& key, const std::string& value) = 0;
    virtual std::string get_parameter(const std::string& key, const std::string& default_value) const = 0;
    virtual void set_parameter(const std::string& key, Config_param* value) = 0;
    virtual Config_param_ref get_parameter(const std::string& key, Config_param_ref default_value = 0) = 0;
    virtual std::string get_base_directory() const = 0;
    virtual std::string get_test_data_directory() const = 0;
};

//
// class Configuration
//

class Configuration : public Object<IConfig> {

protected:
    Config_params params;

public:
    Configuration() {}

    bool parse(int argc, char** argv, const char* options);
    void append(const std::string& key, bool param);
    void append(const std::string& key, int param);
    void append(const std::string& key, const char* param);
    void append(const std::string& key, const std::string& param);
    void append(const std::string& key, Config_param* param);
    void remove(const std::string& key);
    void apply(const std::string& key, bool& value) const;
    void apply(const std::string& key, int& value) const;
    void apply(const std::string& key, std::string& value) const;
    void set_bool_parameter(const std::string& key, bool value);
    int get_bool_parameter(const std::string& key, bool default_value = false) const;
    void set_parameter(const std::string& key, int value);
    int get_parameter(const std::string& key, int default_value) const;
    void set_parameter(const std::string& key, const std::string& value);
    std::string get_parameter(const std::string& key, const std::string& default_value) const;
    void set_parameter(const std::string& key, Config_param* value);
    Config_param_ref get_parameter(const std::string& key, Config_param_ref default_value);
    std::string get_base_directory() const;
    std::string get_test_data_directory() const;

    static std::string read_file(const std::string& filename);
};

}}

#endif
