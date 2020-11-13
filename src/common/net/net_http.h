
//
//  net_http.h
//
//  Created by Christian Lehner on 01/01/17.
//  Copyright (c) 2019 Softhub. All rights reserved.
//

#ifndef NET_HTTP_H
#define NET_HTTP_H

#include "net_socket.h"

#define NET_HTTP_FILE_BASED 0

namespace SOFTHUB {
namespace NET {

FORWARD_CLASS(Http_factory);
FORWARD_CLASS(Http_connection);
FORWARD_CLASS(Http_request);
FORWARD_CLASS(Http_response);
FORWARD_CLASS(Http_header);
FORWARD_CLASS(Http_request_header);
FORWARD_CLASS(Http_response_header);
FORWARD_CLASS(Http_cache);
FORWARD_CLASS(Http_cache_element);

typedef enum {
    invalid_method = 0,
    get_method,
    put_method,
    post_method,
    head_method,
    patch_method
} Http_request_method;

typedef BASE::Hash_map<std::string,std::string> HTTP_custom_parameters;

//
// class Http_factory
//

class Http_factory : public BASE::Object<> {

    static const char* default_user_agent;

    Http_cache_ref cache;
    std::string user_agent;

public:
    Http_factory();
    ~Http_factory();

    Http_cache* get_cache();
    Http_connection_ref create_connection();
    void set_user_agent(const std::string& user_agent) { this->user_agent = user_agent; }
    const std::string& get_user_agent() const { return user_agent; }
};

//
// class Http_connection
//

class Http_connection : public BASE::Object<> {

    friend class Http_factory;

    Http_factory_ref factory;
    int receive_timeout;
    bool ssl_with_fallback;
    std::stringstream data_stream;

    NET::Socket_ref connect(const NET::Url* url);
    NET::Socket* create_socket(const NET::Url* url);
    Http_response_ref redirect(const Http_request* request, Http_response* response);
    bool receive_response(NET::Socket* socket, Http_request_method method, Http_response* response);
    bool retrieve(NET::Socket* socket, const Http_request* request, const NET::Url* url, Http_response* response);

    bool send_request(NET::Socket* socket, const Http_request* request, const NET::Url* url);

    static std::string fix_url_path(const NET::Url* url);

    Http_connection(Http_factory* factory = 0);

public:
    void set_ssl_with_fallback(bool state) { ssl_with_fallback = state; }
    bool get_ssl_with_fallback() const { return ssl_with_fallback; }
    Http_response_ref query(const Http_request* request);
};

#if NET_HTTP_FILE_BASED

//
// class Http_file_connection
//

class Http_file_connection : public BASE::Object<> {

    friend class Http_factory;

    Http_factory_ref factory;
    int receive_timeout;
    bool ssl_with_fallback;

    NET::Socket_ref connect(const NET::Url* url);
    NET::Socket* create_socket(const NET::Url* url);
    Http_response_ref redirect(const Http_request* request, Http_response* response, FILE* file);
    bool receive_response(NET::Socket* socket, Http_request_method method, Http_response* response, FILE* file);
    bool retrieve(NET::Socket* socket, const Http_request* request, const NET::Url* url, Http_response* response, FILE* file);

    bool send_request(NET::Socket* socket, const Http_request* request, const NET::Url* url);

    static std::string fix_url_path(const NET::Url* url);

    Http_file_connection(Http_factory* factory = 0);

public:
    void set_ssl_with_fallback(bool state) { ssl_with_fallback = state; }
    bool get_ssl_with_fallback() const { return ssl_with_fallback; }
    Http_response_ref query(const Http_request* request);
};

#endif

//
// class Http_request
//

class Http_request : public BASE::Object<> {

    Http_request_header_ref header;
    std::string data;
    NET::Url_const_ref url;
    int max_redirects;
    bool caching;

public:
    Http_request(Http_request_method method, const NET::Url* url, int max_redirects = 3);

    Http_request_header* get_header() { return header; }
    const Http_request_header* get_header() const { return header; }
    const NET::Url* get_url() const { return url; }
    void set_data(const std::string& data) { this->data = data; }
    const std::string& get_data() const { return data; }
    int get_max_redirects() const { return max_redirects; }
    void set_caching(bool state) { caching = state; }
    bool get_caching() const { return caching; }
};

//
// class Http_response
//

class Http_response : public BASE::Object<> {

    std::string content;
    Http_response_const_ref parent;
    Http_response_header_const_ref header;

public:
    Http_response(const Http_response* parent = 0);

    void set_content(const std::string& content) { this->content = content; }
    const std::string& get_content() const { return content; }
    const Http_response* get_parent() const { return parent; }
    void set_header(Http_response_header* header) { this->header = header; }
    const Http_response_header* get_header() const { return header; }

    static Http_response_ref error;
};

#if NET_HTTP_FILE_BASED

//
// class Http_file_response
//

class Http_file_response : public BASE::Object<> {

    std::string filepath;
    Http_response_const_ref parent;
    Http_response_header_const_ref header;

public:
    Http_file_response(const std::string& filepath, const Http_response* parent = 0);

    const std::string& get_filepath() const { return filepath; }
    const Http_response* get_parent() const { return parent; }
    void set_header(Http_response_header* header) { this->header = header; }
    const Http_response_header* get_header() const { return header; }

    static Http_response_ref error;
};

#endif

//
// class Http_header
//

class Http_header : public BASE::Object<> {

    std::string version;
    std::string location;
    std::string content_encoding;
    std::string content_language;
    std::string content_type;
    time_t date;
    time_t last_modified;
    int content_length;
    std::string server;
    std::string host;
    std::string accept;
    std::string language;
    std::string user_agent;
    std::string auth_type;
    std::string authorization;

protected:
    void parse_header_attribute(const std::string& line);
    time_t parse_date(const std::string& date);
    int parse_header_attributes(std::stringstream& line_stream);

public:
    Http_header();

    void set_version(const std::string& version) { this->version = version; }
    const std::string& get_version() const { return version; }
    const std::string& get_location() const { return location; }
    void set_location(const std::string& location) { this->location = location; }
    void set_content_encoding(const std::string& encoding) { this->content_encoding = encoding; }
    const std::string& get_content_encoding() const { return content_encoding; }
    void set_content_type(const std::string& content_type) { this->content_type = content_type; }
    const std::string& get_content_type() const { return content_type; }
    void set_content_language(const std::string& language) { this->content_language = language; }
    const std::string& get_content_language() const { return content_language; }
    void set_date(time_t date) { this->date = date; }
    time_t get_date() const { return date; }
    void set_last_modified(time_t date) { this->last_modified = date; }
    time_t get_last_modified() const { return last_modified; }
    void set_content_length(int length) { this->content_length = length; }
    int get_content_length() const { return content_length; }
    void set_server(const std::string& server) { this->server = server; }
    const std::string& get_server() const { return server; }
    void set_host(const std::string& host) { this->host = host; }
    const std::string& get_host() const { return host; }
    void set_accept(const std::string& accept) { this->accept = accept; }
    const std::string& get_accept() const { return accept; }
    void set_language(const std::string& language) { this->language = language; }
    const std::string& get_language() const { return language; }
    void set_user_agent(const std::string& user_agent) { this->user_agent = user_agent; }
    const std::string& get_user_agent() const { return user_agent; }
    void set_auth_type(const std::string& auth_type) { this->auth_type = auth_type; }
    const std::string& get_auth_type() const { return auth_type; }
    void set_authorization(const std::string& authorization) { this->authorization = authorization; }
    const std::string& get_authorization() const { return authorization; }

    bool parse_content_type(std::string& type, std::string& encoding) const;
    bool parse_language(std::string& lang, std::string& region) const;
};

//
// class Http_request_header
//

class Http_request_header : public Http_header {

    static const char* method_name[];

    Http_request_method method;
    std::string path;
    std::string version;
    HTTP_custom_parameters custom_parameters;

public:
    Http_request_header(Http_request_method method = Http_request_method::get_method);

    void set_method(Http_request_method method) { this->method = method; }
    Http_request_method get_method() const { return method; }
    const char* get_method_name() const { return method_name[method]; }
    void set_path(const std::string& path) { this->path = path; }
    const std::string& get_path() const { return path; }
    void set_version(const std::string& version) { this->version = version; }
    const std::string& get_version() const { return version; }
    void add_custom_parameter(const std::string& name, const std::string& value);
    const HTTP_custom_parameters& get_custom_parameters() const;
    const std::string& find_custom_parameter(const std::string& name) const;

    static Http_request_method method_from_string(const std::string& mid);
    static Http_request_header* parse_header(char* buf, int bufsize, int& bytes_remaining);
};

//
// class Http_response_header
//

class Http_response_header : public Http_header {

    std::string response;
    int response_code;

public:
    Http_response_header() : response_code(0) {}

    void set_response(const std::string& response) { this->response = response; }
    const std::string& get_response() const { return response; }
    void set_response_code(int response_code) { this->response_code = response_code; }
    int get_response_code() const { return response_code; }
    bool is_response_ok() const { return 200 <= response_code && response_code < 300; }

    static Http_response_header* parse_header(char* buf, int bufsize, int& bytes_remaining);
};

//
// class Http_cache
//

typedef BASE::Cache<Url_const_ref,Http_cache_element_ref,BASE::Object<> > HTTP_cache_base;

class Http_cache : public HTTP_cache_base {

    static const int default_cache_size = 100;
    static const char* default_cache_location;

    std::string tmp_dir;

public:
    Http_cache();

    void set_tmp_dir(const std::string& dir) { tmp_dir = dir; }
    const std::string& get_tmp_dir() const { return tmp_dir; }
    void clear();
};

//
// class Http_cache_element
//

class Http_cache_element : public BASE::Object<> {

    Url_const_ref url;
    std::string content;

public:
    Http_cache_element(const Url* url) : url(url) {}

    bool is_expired() const { return true; }
    void set_content(const std::string& content) { this->content = content; }
    const std::string& get_content() const { return content; }
    bool erase();
};

#if NET_HTTP_FILE_BASED

//
// class Http_file_cache_element
//

class Http_file_cache_element : public BASE::Object<> {

    Url_const_ref url;
    std::string filepath;

public:
    Http_file_cache_element(const Url* url) : url(url) {}

    bool is_expired() const { return true; }
    void set_filepath(const std::string& filepath) { this->filepath = filepath; }
    const std::string& get_filepath() const { return filepath; }
    bool erase();
};

#endif

}}

#endif
