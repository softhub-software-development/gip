
/*
 *  net_url.cpp
 *
 *  Created by Christian Lehner on 08/14/12.
 *  Copyright 2012 softhub. All rights reserved.
 *
 */

#include "stdafx.h"
#include <hal/hal.h>
#include "net_url.h"
#include <sstream>

using namespace SOFTHUB::BASE;
using namespace SOFTHUB::HAL;
using namespace std;

namespace SOFTHUB {
namespace NET {

static char from_hex_digit(char c);
static char to_hex_digit(char c);

Url::Url(const string& protocol, const string& server, int port, const string& path) :
    protocol(protocol), server(server), path(path), port((ushort) port)
{
    normalize();
}

Url::Url(const Url* base, const string& path) :
    protocol(base->protocol), server(base->server), path(path), port(base->port)
{
    normalize();
}

void Url::normalize()
{
    transform(protocol.begin(), protocol.end(), protocol.begin(), ::tolower);
    transform(server.begin(), server.end(), server.begin(), ::tolower);
#if 0
    // this breaks http redirection
    size_t path_len = path.length();
    if (path_len > 0 && path[path_len-1] == '/')
        path = path.substr(0, path_len-1);
#endif
}

Url* Url::create(const string& s, const string& default_protocol)
{
    static const char* colon_slash_slash = "://";
    string protocol, host, port, path;
    int default_port_no;
    size_t prot_index = s.find(colon_slash_slash);
    size_t host_index;
    if (prot_index != string::npos) {
        protocol = s.substr(0, prot_index);
        host_index = prot_index + strlen(colon_slash_slash);
    } else {
        protocol = default_protocol;
        host_index = 0;
    }
    default_port_no = port_for_protocol(protocol);
    size_t path_index = s.find('/', host_index);
    if (path_index == string::npos)
        path_index = s.find('?', host_index);
    if (path_index == string::npos)
        path_index = s.length();
    size_t host_tail = path_index;
    size_t port_index = s.find(':', host_index);
    if (port_index != string::npos && port_index < path_index) {
        port = s.substr(port_index + 1, path_index - port_index - 1);
        host_tail = port_index;
    }
    size_t host_name_len = host_tail - host_index;
    if (host_name_len > 0)
        host = s.substr(host_index, host_name_len);
    else
        host = "";
    if (path_index != s.length())
        path = s.substr(path_index);
    int port_no = port.length() > 0 ? atoi(port.c_str()) : default_port_no;
    return new Url(protocol, host, port_no, path);
}

Url* Url::create_url(const Url* base, const string& s)
{
    Url* url = 0;
    size_t slen = s.length();
    if (slen == 0)
        return 0;
    size_t hash_pos = s.find('#');
    string clean = hash_pos == string::npos ? s : s.substr(0, hash_pos);
    size_t clean_len = clean.length();
    if (clean_len > 0 && clean[0] == '/') {
        if (clean_len > 1 && clean[1] == '/') {
            url = Url::create("http:" + clean);
        } else {
            url = new Url(base, clean);
        }
    } else {
        if (clean.find("http://") == 0 || clean.find("https://") == 0) {
            url = Url::create(clean);
        } else if (clean.find("mailto:") == 0) {
            url = 0;
        } else if (base) {
            string base_path = base->get_path();
            size_t query_index = base_path.find('?');
            if (query_index == string::npos) {
                size_t slash_index = base_path.rfind('/');
                if (slash_index != string::npos)
                    base_path = base_path.substr(0, slash_index);
            } else {
                base_path = base_path.substr(0, query_index);
            }
            size_t base_path_len = base_path.length();
            if (base_path_len == 0 || base_path[base_path_len-1] != '/')
                base_path += '/';
            string url_path = base_path + clean;
            url = new Url(base, url_path);
        }
    }
    return url;
}

int Url::port_for_protocol(const string& protocol)
{
    if (protocol == "http")
        return 80;
    if (protocol == "https")
        return 443;
    if (protocol == "ftp")
        return 21;
    return 0;
}

string Url::encode(const string& s)
{
    string r;
    size_t i = 0, n = s.length();
    while (i < n) {
        char c = s[i++];
        if (('0' <= c && c <= '9') || ('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z')) {
            r += c;
        } else {
            r += '%';
            r += from_hex_digit((c >> 4) & 0x0f);
            r += from_hex_digit(c & 0x0f);
        }
    }
    return r;
}

string Url::decode(const string& s)
{
    string r;
    size_t i = 0, n = s.length();
    while (i < n) {
        char c = s[i++];
        if (c == '%' && i < n-1) {
            char c0 = s[i++];
            char c1 = s[i++];
            c = to_hex_digit(c0) * 16 + to_hex_digit(c1);
        } else if (c == '+') {
            c = ' ';
        }
        r += c;
    }
    return r;
}

string Url::decode_special(const string& s)
{
    string r;
    size_t i = 0, n = s.length();
    while (i < n) {
        char c = s[i++];
        if (c == '%' && i < n-1) {
            char c0 = s[i++];
            char c1 = s[i++];
            c = to_hex_digit(c0) * 16 + to_hex_digit(c1);
        }
        r += c;
    }
    return r;
}

string Url::domain_name() const
{
#if 0
    size_t idx = server.find_last_of('.');
    if (idx != string::npos)
        idx = server.find_last_of('.', idx-1);
    return idx == string::npos ? server : server.substr(idx+1);
#else
    return server;
#endif
}

string Url::top_level_domain() const
{
    size_t idx = server.find_last_of('.');
    return idx == string::npos ? "" : server.substr(idx+1);
}

string Url::norm_path() const
{
    size_t idx = end_of_path(path);
    return path.substr(0, idx);
}

size_t Url::end_of_path(const string& s)
{
    size_t idx;
    if (Strings::ends_with(s, '/')) {
        size_t slash = s.find_last_not_of('/');
        idx = slash == string::npos ? 0 : slash + 1;
    } else {
        idx = s.length();
    }
    return idx;
}

bool Url::is_same_path(const Url* url) const
{
    const string& other_path = url->get_path();
    size_t i0 = end_of_path(path);
    size_t i1 = end_of_path(other_path);
    return path.compare(0, i0, other_path, 0, i1) == 0;
}

size_t Url::parse_parameters(Url_parameters& parameters) const
{
    return parse_get_parameters(path, parameters);
}

size_t Url::parse_parameters(Url_parameter_map& parameter_map) const
{
    Url_parameters parameters;
    size_t num_params = parse_get_parameters(path, parameters);
    map_parameters(parameters, parameter_map);
    return num_params;
}

size_t Url::parse_parameter(const string& line, Url_parameters& parameters)
{
    size_t eq_pos = line.find('=');
    if (eq_pos == string::npos) {
        Url_parameter param(line, "");
        parameters.append(param);
    } else {
        string key = line.substr(0, eq_pos);
        string val = line.substr(eq_pos + 1);
        Url_parameter param(key, val);
        parameters.append(param);
    }
    return parameters.size();
}

size_t Url::parse_parameters(const string& param_str, Url_parameters& parameters, bool decode_all)
{
    string sub;
    size_t a0 = 0;
    bool done = false;
    while (!done) {
        size_t a1 = param_str.find('&', a0);
        if (a1 == string::npos) {
            sub = param_str.substr(a0);
            done = true;
        } else {
            sub = param_str.substr(a0, a1 - a0);
        }
        size_t eq_pos = sub.find('=');
        if (eq_pos == string::npos) {
            Url_parameter param(sub, "");
            parameters.append(param);
        } else {
            string key = sub.substr(0, eq_pos);
            string val = sub.substr(eq_pos + 1);
            val = decode_all ? Url::decode(val) : Url::decode_special(val);
            Url_parameter param(key, val);
            parameters.append(param);
        }
        a0 = a1 + 1;
    }
    return parameters.size();
}

size_t Url::parse_get_parameters(const string& param_str, Url_parameters& parameters)
{
    size_t param_pos = param_str.find("?");
    if (param_pos == string::npos)
        return 0;
    string path_params = param_str.substr(param_pos + 1);
    return parse_parameters(path_params, parameters, true);
}

size_t Url::parse_post_parameters(const string& param_str, Url_parameters& parameters)
{
    return parse_parameters(param_str, parameters, false);
}

void Url::map_parameters(const Url_parameters& parameters, Url_parameter_map& map)
{
    Url_parameters::const_iterator it = parameters.begin();
    Url_parameters::const_iterator tail = parameters.end();
    while (it != tail) {
        const Url_parameter& param = *it++;
        const string& val = param.second;
        map.insert(param.first, val);
    }
}

void Url::parse_server_name(const string& server, string& host, string& domain, string& tld)
{
    size_t tld_dot_pos = server.rfind('.');
    if (tld_dot_pos == string::npos) {
        tld = server;
    } else {
        tld = server.substr(tld_dot_pos + 1);
        size_t dom_dot_pos = server.rfind('.', tld_dot_pos - 1);
        if (dom_dot_pos == string::npos) {
            domain = server.substr(0, tld_dot_pos);
        } else {
            domain = server.substr(dom_dot_pos + 1, tld_dot_pos - dom_dot_pos - 1);
            host = server.substr(0, dom_dot_pos);
        }
    }
}

string Url::encode(const Url* url)
{
    Url_const_ref base(new Url(url, ""));
    const string& path = url->get_path();
    return base->to_string() + encode(path);
}

string Url::decode(const Url* url)
{
    Url_const_ref base(new Url(url, ""));
    const string& path = url->get_path();
    return base->to_string() + decode(path);
}

void Url::serialize(BASE::Serializer* serializer) const
{
    serializer->write(protocol);
    serializer->write(server);
    serializer->write(path);
    serializer->write(port);
}

void Url::deserialize(BASE::Deserializer* deserializer)
{
    deserializer->read(protocol);
    deserializer->read(server);
    deserializer->read(path);
    deserializer->read(port);
}

bool Url::operator==(const Interface& obj) const
{
    const Url* url = dynamic_cast<const Url*>(&obj);
    return this == url || (url && protocol == url->protocol && server == url->server && port == url->port && is_same_path(url));
}

size_t Url::hash() const
{
    return std::hash<string>()(protocol) + std::hash<string>()(server) + port + std::hash<string>()(norm_path());
}

string Url::to_string() const
{
    stringstream stream(ios_base::out);
    stream << protocol << "://" + server;
    if ((protocol == "http" && port != 80) || (protocol == "https" && port != 443))
        stream << ":" << port;
    stream << path;
    return stream.str();
}

static char from_hex_digit(char c)
{
    if (0 <= c && c <= 9)
        return c + '0';
    if (10 <= c && c <= 15)
        return c - 10 + 'A';
    return c;
}

static char to_hex_digit(char c)
{
    if ('0' <= c && c <= '9')
        return c - '0';
    if ('a' <= c && c <= 'f')
        return c - 'a' + 10;
    if ('A' <= c && c <= 'F')
        return c - 'A' + 10;
    return c;
}

}}

