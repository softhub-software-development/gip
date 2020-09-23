
//
//  net_http.cpp
//
//  Created by Christian Lehner on 01/01/17.
//  Copyright (c) 2019 Softhub. All rights reserved.
//

#include "stdafx.h"
#include "net_http.h"
#include "net_address.h"
#include <hal/hal.h>
#include <iostream>
#include <sstream>

using namespace SOFTHUB::BASE;
using namespace SOFTHUB::HAL;
using namespace std;

namespace SOFTHUB {
namespace NET {

//
// class Http_factory
//

Http_factory::Http_factory() :
    cache(new Http_cache()),
    user_agent(default_user_agent)
{
}

Http_factory::~Http_factory()
{
    cache->clear();
}

Http_cache* Http_factory::get_cache()
{
    return cache;
}

Http_connection_ref Http_factory::create_connection()
{
    return new Http_connection(this);
}

const char* Http_factory::default_user_agent =
#ifdef _DEBUG
    "Weberknecht (still in development, sorry for any inconvenience) +http://www.softhub.com/bot.html";
#else
    "Weberknecht +http://www.softhub.com/bot.html";
#endif

//
// class Http_connection
//

Http_connection::Http_connection(Http_factory* factory) :
    factory(factory), receive_timeout(12000), ssl_with_fallback(false)
{
}

Socket_ref Http_connection::connect(const Url* url)
{
    string host = url->get_server();
    int port = url->get_port();
    Address_const_ref address;
    if (host == "localhost") {
        address = Address_ip4::create_loopback_address(port);
    } else {
        address = Address::create_from_dns_name(host, port);
    }
    if (!address)
        return 0;
    Socket_ref socket(create_socket(url));
    if (!socket)
        return 0;
    socket->set_recieve_timeout(receive_timeout);
    if (socket->connect(address) != SUCCESS)
        return 0;
    return socket;
}

Socket* Http_connection::create_socket(const Url* url)
{
    Socket* socket = 0;
    bool secure = url->get_protocol() == "https";
#if FEATURE_NET_SSL
    if (secure)
        socket = new Socket_tcp_secure_client();
    if (!socket && (!secure || ssl_with_fallback))
        socket = new Socket_tcp();
#else
    if (!secure || ssl_with_fallback)
        socket = new Socket_tcp();
#endif
    return socket;
}

#if NET_HTTP_FILE_BASED
Http_response_ref Http_connection::redirect(const Http_request* request, Http_response* redirect_response, FILE* file)
#else
Http_response_ref Http_connection::redirect(const Http_request* request, Http_response* redirect_response)
#endif
{
    const Url* url = request->get_url();
    Http_response_ref response = redirect_response;
    const Http_response_header* header = response->get_header();
    int code = header->get_response_code(), redirect_count = 0;
    while ((code == 301 || code == 302) && redirect_count++ < request->get_max_redirects()) {
        const string& location = header->get_location();
        Url_ref redirect_url = Url::create(location);
        if (redirect_url->get_protocol() == "file")
            redirect_url = new Url(url, redirect_url->get_path());
        Socket_ref socket = connect(redirect_url);
        if (!socket)
            return Http_response::error;
#if NET_HTTP_FILE_BASED
        const string& tmp_name = response->get_filepath();
        response = new Http_response(tmp_name, redirect_response);
        retrieve(socket, request, redirect_url, response, file);
#else
        data_stream.str("");
        retrieve(socket, request, redirect_url, response);
        const string& data = data_stream.str();               // TODO: move data_stream variable into response
        response = new Http_response(redirect_response);
        response->set_content(data);
#endif
        header = response->get_header();
        code = header->get_response_code();
#ifdef _DEBUG
        cout << "redirected " << url->to_string() << " to " << location << " [" << code << "]" << endl;
#endif
        socket->close();
    }
    return response;
}

Http_response_ref Http_connection::query(const Http_request* request)
{
    Http_cache* cache = factory->get_cache();
    bool use_cache = cache && request->get_caching();
    const Url* url = request->get_url();
    Http_response_ref response = 0;
    Http_cache_element_ref element;
    if (use_cache)
        element = cache->find(url);
    if (!element || element->is_expired()) {
        Socket_ref socket = connect(url);
        if (!socket)
            return Http_response::error;
        const Http_request_header* request_header = request->get_header();
        Http_request_method method = request_header->get_method();
        element = new Http_cache_element(url);
#if NET_HTTP_FILE_BASED
        const string& tmp_dir = cache->get_tmp_dir();
        string tmp_template = tmp_dir + "page-XXXXXX";
        char* tmp_name = (char*) tmp_template.c_str();
        FILE* tmp_file = 0;
        if (method != head_method) {
            int tmp_file_des = mkstemp(tmp_name);
            if (tmp_file_des < 0)
                return 0;
            tmp_file = fdopen(tmp_file_des, "w");
            if (!tmp_file)
                return 0;
        }
        string filename = File_path::filename_of(tmp_template);
        if (filename.length() == 0)
            return 0;
        string filepath = tmp_dir + filename;
        element->set_filepath(filepath);
        response = new Http_response(filepath);
        retrieve(socket, request, url, response, tmp_file);
#else
        response = new Http_response();
        retrieve(socket, request, url, response);
        const string& data = data_stream.str();     // TODO: move this to response
        response->set_content(data);
        element->set_content(data);
#endif
        socket->close();
        const Http_response_header* response_header = response->get_header();
        if (!response_header) {
#if NET_HTTP_FILE_BASED
            if (tmp_file)
                fclose(tmp_file);
#else
#endif
            return Http_response::error;
        }
        int code = response_header->get_response_code();
        switch (code) {
        case 301:
        case 302:
#if NET_HTTP_FILE_BASED
            if (method != head_method)
                tmp_file = freopen(tmp_name, "w", tmp_file);
            response = redirect(request, response, tmp_file);
#else
            response = redirect(request, response);
#endif
            break;
        case 403:
            break;
        }
        if (use_cache && method != head_method)
            cache->store(url, element);
#if NET_HTTP_FILE_BASED
        if (method != head_method)
            fclose(tmp_file);
#else
#endif
    } else {
#if NET_HTTP_FILE_BASED
        const string& filepath = element->get_filepath();
        response = new Http_response(filepath);
#else
        const string& content = element->get_content();
        response = new Http_response();
        response->set_content(content);
#endif
    }
    return response;
}

bool Http_connection::send_request(NET::Socket* socket, const Http_request* request, const Url* url)
{
    const Http_request_header* header = request->get_header();
    const string& path = fix_url_path(url);
    const string& host = url->get_server();
    string http_version = header->get_version();
    if (http_version.empty())
        http_version = "HTTP/1.0";
    const string& method_str = header->get_method_name();
    const string& user_agent = factory->get_user_agent();
    const string& data = request->get_data();
    stringstream stream;
    stream << method_str << " " << path << " " << http_version << "\r\n";
    stream << "Host: " << host << "\r\n";
    stream << "User-agent: " << user_agent << "\r\n";
    string accept = header->get_accept();
    if (accept.empty())
        accept = "text/html;q=0.9,*/*;q=0.8";
    stream << "Accept: " << accept << "\r\n";
    string encoding = header->get_content_encoding();
    if (encoding.empty())
        encoding = "text/plain; charset=utf-8";
    stream << "Content-Type: " << encoding << "\r\n";
    const string& authorization = header->get_authorization();
    if (!authorization.empty()) {
        const string& auth_type = header->get_auth_type();
        const string& atype = auth_type.empty() ? "Basic" : auth_type;
        stream << "Authorization: " << atype << " " << authorization << "\r\n";
    }
    string language = header->get_language();
    if (!language.empty())
        stream << "Language: " << language << "\r\n";
    const HTTP_custom_parameters& parameters = header->get_custom_parameters();
    HTTP_custom_parameters::const_iterator it = parameters.begin();
    HTTP_custom_parameters::const_iterator tail = parameters.end();
    while (it != tail) {
        const HTTP_custom_parameters::value_type& val = *it++;
        stream << val.first << ": " << val.second << "\r\n";
    }
    // TOOD: use all attributes from header
    stream << "Content-Length: " << data.length() << "\r\n";
    stream << "Cache-Control: max-age=0\r\n\r\n";
    // TOOD: send data
    if (!data.empty())
        stream << data;
    const string header_str = stream.str();
#ifdef _DEBUG
    cout << "sending request: " << url->to_string() << endl << header_str << endl;
#endif
    const char* header_text = header_str.c_str();
    int header_text_len = (int) header_str.length();
    int n, count = 0;
    do {
        n = socket->send(header_text, header_text_len);
    } while (n > 0 && (count += n) < header_text_len);
    assert(count <= header_text_len);
    return count == header_text_len;
}

#if NET_HTTP_FILE_BASED
bool Http_connection::receive_response(Socket* socket, Http_request_method method, Http_response* response, FILE* file)
{
    const int max_buf_size = 4096;
    char buf[max_buf_size];
    bool in_content = false;
    int count;
    while ((count = socket->recv(buf, max_buf_size)) > 0) {
        if (in_content) {
            fwrite(buf, 1, count, file);
        } else {
            int bytes_remaining = 0;
            Http_response_header* header = Http_response_header::parse_header(buf, count, bytes_remaining);
            response->set_header(header);
//          assert(0 <= bytes_remaining && bytes_remaining <= count);
            if (method == head_method)
                break;
            if (bytes_remaining > 0) {
                int offset = count - bytes_remaining;
                assert(0 <= offset && offset < max_buf_size);
                fwrite(buf + offset, 1, bytes_remaining, file);
            }
            in_content = true;
        }
    }
    return count >= 0;
}
#else
bool Http_connection::receive_response(Socket* socket, Http_request_method method, Http_response* response)
{
    const int max_buf_size = 4096;
    char buf[max_buf_size];
    bool in_content = false;
    int count;
    while ((count = socket->recv(buf, max_buf_size)) > 0) {
        if (in_content) {
            data_stream.write(buf, count);
        } else {
            int bytes_remaining = 0;
            Http_response_header* header = Http_response_header::parse_header(buf, count, bytes_remaining);
            response->set_header(header);
//          assert(0 <= bytes_remaining && bytes_remaining <= count);
            if (method == head_method)
                break;
            if (bytes_remaining > 0) {
                int offset = count - bytes_remaining;
                assert(0 <= offset && offset < max_buf_size);
                data_stream.write(buf + offset, bytes_remaining);
            }
            in_content = true;
        }
    }
    return count >= 0;
}
#endif

#if NET_HTTP_FILE_BASED
bool Http_connection::retrieve(Socket* socket, const Http_request* request, const Url* url, Http_response* response, FILE* file)
#else
bool Http_connection::retrieve(Socket* socket, const Http_request* request, const Url* url, Http_response* response)
#endif
{
    if (!send_request(socket, request, url))
        return false;
    const Http_request_header* request_header = request->get_header();
    Http_request_method method = request_header->get_method();
#if NET_HTTP_FILE_BASED
    return receive_response(socket, method, response, file);
#else
    return receive_response(socket, method, response);
#endif
}

string Http_connection::fix_url_path(const NET::Url* url)
{
    string path = url->get_path();
    if (path.empty()) {
        path += '/';
    } else {
        size_t hash_pos = path.find('#');
        if (hash_pos != string::npos)
            path = path.substr(0, hash_pos);
    }
    return path;
}

//
// class Http_request
//

Http_request::Http_request(Http_request_method method, const NET::Url* url, int max_redirects) :
    header(new Http_request_header(method)), url(url), max_redirects(max_redirects), caching(false)
{
}

//
// class Http_response
//

Http_response_ref Http_response::error = new Http_response();

#if NET_HTTP_FILE_BASED
Http_response::Http_response(const string& filepath, const Http_response* parent) :
    filepath(filepath), parent(parent), header(parent ? parent->get_header() : 0)
#else
Http_response::Http_response(const Http_response* parent) :
    parent(parent), header(parent ? parent->get_header() : 0)
#endif
{
}

//
// class Http_header
//

Http_header::Http_header() :
    date(0), last_modified(0), content_length(0)
{
}

bool Http_header::parse_content_type(string& type, string& encoding) const
{
    BASE::String_vector sv;
    Strings::split(content_type, sv, ";", true);
    switch (sv.size()) {
    case 0:
        return false;
    case 1:
        type = sv[0];
        return true;
    default:
        type = sv[0];
        encoding = sv[1];
        return true;
    }
}

bool Http_header::parse_language(string& lang, string& region) const
{
    BASE::String_vector sv;
    Strings::split(language, sv, "-", true);
    switch (sv.size()) {
    case 0:
        return false;
    case 1:
        lang = sv[0];
        return true;
    default:
        lang = sv[0];
        region = sv[1];
        return true;
    }
}

time_t Http_header::parse_date(const string& date)
{
    time_t t = 0;
    struct tm time_spec;
    memset(&time_spec, 0, sizeof(struct tm));
    if (strptime(date.c_str(), "%a, %d %b %Y %H:%M:%S", &time_spec))
        t = mktime(&time_spec);
    return t;
}

void Http_header::parse_header_attribute(const string& line)
{
    stringstream stream(line);
    string attr;
    stream >> attr;
    Strings::to_lower(attr);
    if (attr == "content-encoding:") {
        string encoding;
        getline(stream, encoding);
        Strings::trim(encoding);
        set_content_encoding(encoding);
    } else if (attr == "content-language:") {
        string language;
        getline(stream, language);
        Strings::trim(language);
        set_content_language(language);
    } else if (attr == "date:") {
        string date;
        getline(stream, date);
        Strings::trim(date);
        time_t t = parse_date(date);
        set_date(t);
    } else if (attr == "last-modified:") {
        string date;
        getline(stream, date);
        Strings::trim(date);
        time_t t = parse_date(date);
        set_last_modified(t);
    } else if (attr == "content-type:") {
        string content_type;
        stream >> content_type;
        Strings::trim(content_type);
        set_content_type(content_type);
    } else if (attr == "content-length:") {
        int len;
        stream >> len;
        set_content_length(len);
    } else if (attr == "location:") {
        string location;
        stream >> location;
        Strings::trim(location);
        set_location(location);
    } else if (attr == "server:") {
        string server;
        stream >> server;
        Strings::trim(server);
        set_server(server);
    } else if (attr == "host:") {
        string host;
        stream >> host;
        Strings::trim(host);
        set_host(host);
    } else if (attr == "accept:") {
        string format;
        stream >> format;
        Strings::trim(format);
        set_accept(format);
    } else if (attr == "accept-language:") {
        string language;
        stream >> language;
        Strings::trim(language);
        set_language(language);
    } else if (attr == "user-agent:") {
        size_t off = attr.length();
        string user_agent = line.substr(off);
        Strings::trim(user_agent);
        set_user_agent(user_agent);
    } else if (attr == "authorization:") {
        // TODO: Authorization: Basic YWxhZGRpbjpvcGVuc2VzYW1l
        string auth_type, authorization;
        stream >> auth_type >> authorization;
        set_auth_type(auth_type);
        set_authorization(authorization);
    }
}

int Http_header::parse_header_attributes(stringstream& line_stream)
{
    int count = 0;
    string line;
    while (getline(line_stream, line)) {
        count += (int) line.length() + 1;
        Strings::trim(line);
        if (line.empty())
            break;
        parse_header_attribute(line);
    }
    return count;
}

//
// class Http_request_header
//

const char* Http_request_header::method_name[] = { "<INVALID>", "GET", "PUT", "POST", "HEAD", "PATCH" };

Http_request_header::Http_request_header(Http_request_method method) : method(method)
{
}

Http_request_header* Http_request_header::parse_header(char* buf, int bufsize, int& bytes_remaining)
{
    // TODO: this only works if header fits into max_buf_size byte buffer
    Http_request_header* header = 0;
    int count = 0;
    string line;
    string text(buf, bufsize);
    stringstream line_stream(text);
    if (getline(line_stream, line)) {
        count += (int) line.length() + 1;
        Strings::trim(line);
        if (line.empty())
            return 0;
        header = new Http_request_header();
        stringstream stream(line);
        string method_name, path, version;
        stream >> method_name >> path >> version;
        Http_request_method method = method_from_string(method_name);
        header->set_method(method);
        header->set_path(path);
        header->set_version(version);
        count += header->parse_header_attributes(line_stream);
    }
    bytes_remaining = bufsize - count;
    return header;
}

void Http_request_header::add_custom_parameter(const string& name, const string& value)
{
    custom_parameters.insert(name, value);
}

const HTTP_custom_parameters& Http_request_header::get_custom_parameters() const
{
    return custom_parameters;
}

const string& Http_request_header::find_custom_parameter(const string& name) const
{
    return custom_parameters.get(name);
}

Http_request_method Http_request_header::method_from_string(const string& mid)
{
    for (int i = 0, n = sizeof(method_name) / sizeof(const char*); i < n; i++)
        if (mid == method_name[i])
            return (Http_request_method) i;
    stringstream stream;
    stream << "invalid method " << mid;
    throw Exception(stream.str());
}

//
// class Http_response_header
//

Http_response_header* Http_response_header::parse_header(char* buf, int bufsize, int& bytes_remaining)
{
    // TODO: this only works if header fits into max_buf_size byte buffer
    Http_response_header* header = 0;
    int count = 0;
    string line;
    string text(buf, bufsize);
    stringstream line_stream(text);
    if (getline(line_stream, line)) {
        count += (int) line.length() + 1;
        Strings::trim(line);
        if (line.empty())
            return 0;
        header = new Http_response_header();
        stringstream stream(line);
        string attr;
        stream >> attr;
        Strings::to_lower(attr);
        if (attr.find("http/") == 0) {
            int code;
            string status;
            stream >> code >> status;
            Strings::trim(line);
            header->set_response(line);
            header->set_response_code(code);
            count += header->parse_header_attributes(line_stream);
        }
    }
    bytes_remaining = bufsize - count;
    return header;
}

//
// class Http_cache
//

Http_cache::Http_cache() : HTTP_cache_base(default_cache_size), tmp_dir(default_cache_location)
{
}

void Http_cache::clear()
{
    const_iterator it = container.begin();
    const_iterator tail = container.end();
    while (it != tail) {
        const HTTP_cache_base::value_type& value = *it++;
        Http_cache_element_ref element = value.second;
        bool success = element->erase();
        if (!success)
            log_message(WARN, "failed to clear cache");
    }
    HTTP_cache_base::clear();
}

//
// class Http_cache_element
//

bool Http_cache_element::erase()
{
#if NET_HTTP_FILE_BASED
    return File_path::remove_file(filepath);
#else
    return true;
#endif
}

const char* Http_cache::default_cache_location = "/var/tmp/";

}}

