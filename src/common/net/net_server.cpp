
//
//  net_server.cpp
//
//  Created by Christian Lehner on 21/05/15.
//  Copyright (c) 2019 Softhub. All rights reserved.
//

#include "stdafx.h"
#include "net_server.h"

#define DOCUMENT_ROOT "/var/www"

using namespace SOFTHUB::BASE;
using namespace SOFTHUB::HAL;
using namespace std;

namespace SOFTHUB {
namespace NET {

//
// class Http_server
//

NET::Address_const_ref Http_server::default_server_address = Address::create(default_port);

Http_server::Http_server() :
    Server(default_server_address), user_agent("Softhub"), send_timeout(12000), receive_timeout(12000), http_port(default_port), use_ssl(false)
{
}

Http_server::~Http_server()
{
    finalize();
}

void Http_server::configure(const IConfig* config)
{
    const string& url_str = config->get_parameter("server-url", "http://localhost:8081");
    int port = config->get_parameter("server-http-port", 0);
    const Url_ref url(Url::create(url_str));
    document_root = config->get_parameter("document-root", DOCUMENT_ROOT);
    use_ssl = url->get_protocol() == "https";
    http_port = port ? port : url->get_port();
    Address* address = Address::create(http_port);
    set_server_address(address);
}

Socket_tcp* Http_server::create_socket() const
{
#if FEATURE_NET_SSL
    return use_ssl ? new Socket_tcp_secure_server() : new Socket_tcp();
#else
    return new Socket_tcp();
#endif
}

bool Http_server::initialize()
{
    if (!server_socket)
        server_socket = create_socket();
    server_socket->reusable(true);
    server_socket->no_sig_pipe(1);
    Status status = server_socket->bind(server_address);
    if (status != SUCCESS) {
        stringstream stream;
        stream << "failed to bind server socket to " << server_address->to_string() << " errno: " << errno;
        report_error(stream.str());
        Thread::sleep(1000);
        return false;
    }
    status = server_socket->listen();
    if (status != SUCCESS) {
        report_error("failed to listen on server socket");
        Thread::sleep(1000);
        return false;
    }
    stringstream stream;
    stream << "server bound to " << server_address->to_string();
    report_error(stream.str());
    return true;
}

void Http_server::finalize()
{
    if (server_socket) {
        server_socket->close();
        server_socket = 0;
    }
}

void Http_server::stop()
{
    if (server_socket)
        server_socket->close();
    Server::stop();
}

void Http_server::serve_request()
{
    if (!server_socket)
        return;
    Address_ref client(new Address_ip4());
    Socket_tcp_ref socket;
    Status status = server_socket->accept(client, socket);
    if (status == SUCCESS && socket) {
        serve(client, socket);
    } else {
        server_socket->close();
    }
}

void Http_server::serve(const Address* client, Socket_tcp* socket)
{
    Http_service_request_ref request(new Http_service_request(this, client, socket));
    if (request->parse_url_parameters()) {
        Http_service_response_ref response(new Http_service_response());
        serve_page(request, response);
        const string& content = response->get_content();
        if (content.length() > 0) {
            socket->set_send_timeout(send_timeout);
            socket->set_recieve_timeout(receive_timeout);
            Status status = send(content, socket);
            if (status != SUCCESS) {
                stringstream stream;
                stream << "failed to send, status: " << status;
                log_message(INFO, stream.str());
            }
        }
    } else {
        log_message(INFO, "failed to parse request parameters");
    }
    // do we need flag to keep sockets open?
    socket->close();
}

Status Http_server::send(const string& msg, Socket_tcp* socket)
{
    int count, response_len = (int) msg.length();
    char* response_buf = (char*) msg.c_str();
    while ((count = socket->send(response_buf, response_len)) > 0) {
        response_buf += count;
        response_len -= count;
    }
    return count >= 0 ? SUCCESS : SEND_ERR;
}

void Http_server::serve_header(const string& status, const string& content_type, size_t content_length, ostream& stream)
{
    time_t now;
    time(&now);
    stream << "HTTP/1.0 " << status << endl;
    stream << "Date: " << formatted_date(now) << endl;
    stream << "Server: " << user_agent << endl;
    stream << "Content-Type: " << content_type << endl;
    stream << "Content-Length: " << content_length << endl;
    stream << "Connection: close" << endl;
}

void Http_server::serve_error_page_content(const string& msg, ostream& stream)
{
    stream << "<!doctype html>" << endl;
    stream << "<html><head>" << endl;
    stream << "  <meta charset='UTF-8'>" << endl;
    stream << "</head><body>" << endl;
    stream << "  Invalid " << user_agent << " request: " << msg << endl;
    stream << "</body></html>" << endl;
}

void Http_server::serve_error_page(const string& msg, Http_service_response* sres)
{
    stringstream stream;
    stream << "error response: " << msg;
    log_message(INFO, stream.str());
    stringstream content_stream;
    serve_error_page_content(msg, content_stream);
    const string& content = content_stream.str();
    stringstream page_stream;
    serve_header("404 Not Found", "text/html", content.length(), page_stream);
    page_stream << endl;
    page_stream << content;
    sres->set_content(page_stream.str());
}

bool Http_server::user_agent_is_mobile(const std::string& user_agent)
{
    string norm_user_agent = user_agent;
    Strings::to_lower(norm_user_agent);
    if (norm_user_agent.find("android") != string::npos)
        return true;
    if (norm_user_agent.find("webos") != string::npos)
        return true;
    if (norm_user_agent.find("iphone") != string::npos)
        return true;
    if (norm_user_agent.find("ipad") != string::npos)
        return true;
    if (norm_user_agent.find("ipod") != string::npos)
        return true;
    if (norm_user_agent.find("blackberry") != string::npos)
        return true;
    if (norm_user_agent.find("windows phone") != string::npos)
        return true;
    if (norm_user_agent.find("iemobile") != string::npos)
        return true;
    if (norm_user_agent.find("nokia") != string::npos)
        return true;
    if (norm_user_agent.find("opera mini") != string::npos)
        return true;
    return false;
}

void Http_server::parse_optional_parameters(const Url_parameters& parameters, string& language, bool& mobile, string& address)
{
    for (size_t i = 1, n = parameters.size(); i < n; i++) {
        const Url_parameter& param = parameters[i];
        if (param.first == "l")
            language = param.second;
        else if (param.first == "m")
            mobile = param.second != "0" && param.second != "false";
        else if (param.first == "a")
            address = param.second;
    }
}

string Http_server::formatted_date(time_t t)
{
    char buf[128];
    const struct tm* ts = localtime(&t);
    strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S GMT", ts);
    // return time in this format "Date: Mon, 22 Apr 2019 11:55:00 GMT"
    return buf;
}

//
// class Http_service_request
//

Http_service_request::Http_service_request(Http_server* server, const Address* client, Socket_tcp* socket) :
    server(server), bytes_read(0), bytes_remaining(0), parameters(0), parameter_map(0), client(client), socket(socket)
{
}

string Http_service_request::get_data() const
{
    int idx = max_buf_size - bytes_remaining;
    int n = bytes_read - idx;
    assert(n >= 0);
    return string(&buffer[idx], n);
}

string Http_service_request::get_user_ip() const
{
    string user = parameter_map->get("a");
    if (user.empty()) {
        const Address* client = get_client();
        user = client->to_string(false);
    }
    return user;
}

const Url_parameters& Http_service_request::get_parameters() const
{
    if (!parameters) {
        parameters = new Url_parameters();
        switch (header->get_method()) {
        case Http_request_method::get_method:
            Url::parse_get_parameters(header->get_path(), *parameters);
            break;
        case Http_request_method::post_method:
            if (header->get_content_type() != "application/json")
                parse_post_parameters();
            break;
        default:
            break;
        }
    }
    return *parameters;
}

const Url_parameter_map& Http_service_request::get_parameter_map() const
{
    if (!parameter_map) {
        parameter_map = new Url_parameter_map();
        const Url_parameters& params = get_parameters();
        Url::map_parameters(params, *parameter_map);
    }
    return *parameter_map;
}

bool Http_service_request::parse_url_parameters()
{
    int retry_cnt = 0;
    while (++retry_cnt < 3 && (bytes_read = socket->recv(buffer, max_buf_size)) == 0);
    if (bytes_read < 0) {
        stringstream stream;
        stream << "bad request from " << client->to_string() << " retries: " << retry_cnt << endl;
        log_message(INFO, stream.str());
        return false;
    }
    header = Http_request_header::parse_header(buffer, max_buf_size, bytes_remaining);
    return true;
}

bool Http_service_request::parse_post_parameters() const
{
    int count = 0;
    int idx = max_buf_size - bytes_remaining;
    int n = bytes_read - idx;
    assert(n >= 0);
    if (n == 0) {
        // TODO: loop on bytes_remaining
        bytes_read = socket->recv(buffer, max_buf_size);
        if (bytes_read <= 0)
            return false;
        idx = 0;
        n = bytes_remaining = bytes_read;
    }
    string line;
    string text(&buffer[idx], n);
    stringstream line_stream(text);
    while (getline(line_stream, line)) {
        count += (int) line.length() + 1;
        Strings::trim(line);
        if (!line.empty())
            Url::parse_post_parameters(line, *parameters);
    }
    return true;
}

//
// class Http_service_response
//

Http_service_response::Http_service_response()
{
}

}}
