
//
//  net_server.h
//
//  Created by Christian Lehner on 21/05/15.
//  Copyright (c) 2019 Softhub. All rights reserved.
//

#ifndef NET_SERVER_H
#define NET_SERVER_H

#include "net_address.h"
#include "net_http.h"
#include <hal/hal.h>

namespace SOFTHUB {
namespace NET {

FORWARD_CLASS(Server);
FORWARD_CLASS(Http_server);
FORWARD_CLASS(Http_config);
FORWARD_CLASS(Http_service_request);
FORWARD_CLASS(Http_service_response);

typedef BASE::List<Server_ref> Server_list;

//
// class Server
//

class Server : public HAL::Service<> {

protected:
    Address_const_ref server_address;

public:
    Server(const Address* address) : server_address(address) {}

    void set_server_address(const Address* address) { server_address = address; }
    const Address* get_server_address() const { return server_address; }
};

//
// class Http_server
//

class Http_server : public Server {

    std::string user_agent;
    std::string document_root;
    int send_timeout;
    int receive_timeout;
    int http_port;
    bool use_ssl;

    Socket_tcp* create_socket() const;

public:
    static const int default_port = 8080;
    static Address_const_ref default_server_address;

protected:
    Socket_tcp_ref server_socket;

    bool initialize();
    void finalize();
    void serve_request();

    void set_user_agent(const std::string& user_agent) { this->user_agent = user_agent; }
    const std::string& get_user_agent() const { return user_agent; }
    const std::string& get_document_root() const { return document_root; }

    Status send(const std::string& msg, Socket_tcp* socket);

    static bool user_agent_is_mobile(const std::string& user_agent);
    static void parse_optional_parameters(const Url_parameters& parameters, std::string& language, bool& mobile, std::string& address);
    static std::string formatted_date(time_t t);

public:
    Http_server();
    ~Http_server();

    void configure(const BASE::IConfig* config);
    void stop();

    virtual void serve(const Address* client, Socket_tcp* socket);
    virtual void serve_header(const std::string& status, const std::string& content_type, size_t content_length, std::ostream& stream);
    virtual void serve_page(const Http_service_request* sreq, Http_service_response* sres) = 0;
    virtual void serve_error_page(const std::string& msg, Http_service_response* sres);
    virtual void serve_error_page_content(const std::string& msg, std::ostream& stream);
};

//
// class Http_service_request
//

class Http_service_request : public BASE::Object<> {

    static const int max_buf_size = 4096;

    Http_server_ref server;
    mutable char buffer[max_buf_size];  // TODO: this must become variable
    mutable int bytes_read;
    mutable int bytes_remaining;
    mutable Http_request_header_ref header;
    mutable Url_parameters* parameters;
    mutable Url_parameter_map* parameter_map;
    Address_const_ref client;
    mutable Socket_tcp_ref socket;

    void parse_url_parameters();
    void parse_post_parameters() const;

public:
    Http_service_request(Http_server* server, const Address* client, Socket_tcp* socket);

    const Http_request_header* get_header() const { return header; }
    std::string get_data() const;
    const Url_parameters& get_parameters() const;
    const Url_parameter_map& get_parameter_map() const;
    std::string get_user_ip() const;
    const Address* get_client() const { return client; }
    Socket_tcp* get_socket() { return socket; }
};

//
// class Http_service_response
//

class Http_service_response : public BASE::Object<> {

    std::string content;

public:
    Http_service_response();

    void set_content(const std::string& content) { this->content = content; }
    const std::string& get_content() const { return content; }
    std::string& get_content() { return content; } // TODO: refactor and remove this
};

}}

#endif
