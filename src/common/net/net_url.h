
/*
 *  net_url.h
 *
 *  Created by Christian Lehner on 08/14/12.
 *  Copyright 2012 softhub. All rights reserved.
 *
 */

#ifndef UTIL_URL_H
#define UTIL_URL_H

#include <base/base.h>
#include <string>

namespace SOFTHUB {
namespace NET {

FORWARD_CLASS(Url);
DECLARE_ARRAY(Url_const_ref, Urls);

typedef std::pair<std::string,std::string> Url_parameter;
typedef BASE::Vector<Url_parameter> Url_parameters;
typedef BASE::Hash_map<std::string,std::string> Url_parameter_map;

class Url : public BASE::Object<> {

    std::string protocol;
    std::string server;
    std::string path;
    ushort port;

    void normalize();

    static size_t end_of_path(const std::string& s);

public:
    Url() : port(0) {}
    Url(const std::string& protocol, const std::string& server, int port, const std::string& path);
    Url(const Url* base, const std::string& path);

    const std::string& get_protocol() const { return protocol; }
    const std::string& get_server() const { return server; }
    int get_port() const { return port; }
    const std::string& get_path() const { return path; }
    std::string domain_name() const;
    std::string top_level_domain() const;
    std::string norm_path() const;
    bool is_same_path(const Url* url) const;
    size_t parse_parameters(Url_parameters& parameters) const;
    size_t parse_parameters(Url_parameter_map& parameter_map) const;
    void serialize(BASE::Serializer* serializer) const;
    void deserialize(BASE::Deserializer* deserializer);
    bool operator==(const Interface& obj) const;
    size_t hash() const;
    std::string to_string() const;

    static Url* create(const std::string& s, const std::string& default_protocol = "file");
    static Url* create_url(const Url* base, const std::string& s);
    static int port_for_protocol(const std::string& protocol);
    static std::string encode(const std::string& s);
    static std::string decode(const std::string& s);
    static std::string decode_special(const std::string& s);
    static std::string encode(const Url* url);
    static std::string decode(const Url* url);
    static size_t parse_parameter(const std::string& line, Url_parameters& parameters);
    static size_t parse_parameters(const std::string& param_str, Url_parameters& parameters, bool decode_all);
    static size_t parse_get_parameters(const std::string& param_str, Url_parameters& parameters);
    static size_t parse_post_parameters(const std::string& param_str, Url_parameters& parameters);
    static void map_parameters(const Url_parameters& parameters, Url_parameter_map& map);
    static void parse_server_name(const std::string& server, std::string& host, std::string& domain, std::string& tld);

    DECLARE_CLASS('surl');
};

}}

#endif

