
//
//  net_socket.cpp
//
//  Created by Christian Lehner on 20/04/16.
//  Copyright (c) 2019 Softhub. All rights reserved.
//

#include "stdafx.h"
#include "net_socket.h"
#include "net_module.h"
#include "net_address.h"
#include <fcntl.h>

#ifdef PLATFORM_WIN
#include <Ws2tcpip.h>
#else
#include <ifaddrs.h>
#define INVALID_SOCKET -1
#define BOOL int
#define FALSE 0
#define TRUE 1
#endif

using namespace std;

namespace SOFTHUB {
namespace NET {

//
// class Socket
//

Socket::Socket() : socket(INVALID_SOCKET)
#ifdef PLATFORM_LINUX
  , send_flags(0)
#endif
{
}

Socket::Socket(int family, int type, int protocol) : socket(::socket(family, type, protocol))
#ifdef PLATFORM_LINUX
  , send_flags(0)
#endif
{
}

Socket::Socket(SOCKET socket) : socket(socket)
#ifdef PLATFORM_LINUX
  , send_flags(0)
#endif
{
}

Socket::~Socket()
{
    if (socket != INVALID_SOCKET)
        close();
}

Status Socket::bind(const Address* address)
{
    assert(socket != INVALID_SOCKET);
    const struct sockaddr& addr = address->get_addr();
    int addr_len = address->get_addr_len();
    int result = ::bind(socket, &addr, addr_len);
#ifdef PLATFORM_WIN
    if (result == SOCKET_ERROR)
        result = WSAGetLastError();
#endif
    return result == 0 ? SUCCESS : BIND_ERR;
}

Status Socket::listen(int back_log)
{
    assert(socket != INVALID_SOCKET);
    int result = ::listen(socket, back_log);
#ifdef PLATFORM_WIN
    if (result == SOCKET_ERROR)
        result = WSAGetLastError();
#endif
    return result == 0 ? SUCCESS : LISTEN_ERR;
}

Status Socket::connect(const Address* address)
{
    assert(socket != INVALID_SOCKET);
    const struct sockaddr& addr = address->get_addr();
    int addr_len = address->get_addr_len();
    int result = ::connect(socket, &addr, addr_len);
#ifdef PLATFORM_WIN
    if (result == SOCKET_ERROR)
        result = WSAGetLastError();
#endif
    return result == 0 ? SUCCESS : CONNECT_ERR;
}

Status Socket::reusable(bool state)
{
    assert(socket != INVALID_SOCKET);
    BOOL flag = state ? TRUE : FALSE;
    char* buf = reinterpret_cast<char*>(&flag);
    int buf_len = sizeof(BOOL);
    int result = ::setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, buf, buf_len);
#ifdef PLATFORM_WIN
    if (result == SOCKET_ERROR)
        result = WSAGetLastError();
#endif
    return result == 0 ? SUCCESS : OPTION_ERR;
}

Status Socket::no_sig_pipe(int state)
{
    assert(socket != INVALID_SOCKET);
    int result = 0;
#ifdef PLATFORM_LINUX
    if (state)
        send_flags |= MSG_NOSIGNAL;
    else
        send_flags &= ~MSG_NOSIGNAL;
#elif defined PLATFORM_MAC
    result = ::setsockopt(socket, SOL_SOCKET, SO_NOSIGPIPE, &state, sizeof(state));
#elif defined PLATFORM_WIN
    // TODO: SO_NOSIGPIPE not defined on Windows
    if (result == SOCKET_ERROR)
        result = WSAGetLastError();
#endif
    return result == 0 ? SUCCESS : OPTION_ERR;
}

Status Socket::set_send_timeout(int msec)
{
#ifdef PLATFORM_WIN
    DWORD tv = msec;
#else
    struct timeval tv;
    tv.tv_sec = msec / 1000;
    tv.tv_usec = msec % 1000 * 1000;
#endif
    int result = setsockopt(socket, SOL_SOCKET, SO_SNDTIMEO, (char*) &tv, sizeof tv);
    return result == 0 ? SUCCESS : OPTION_ERR;
}

Status Socket::set_recieve_timeout(int msec)
{
#ifdef PLATFORM_WIN
    DWORD tv = msec;
#else
    struct timeval tv;
    tv.tv_sec = msec / 1000;
    tv.tv_usec = msec % 1000 * 1000;
#endif
    int result = setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, (char*) &tv, sizeof tv);
    return result == 0 ? SUCCESS : OPTION_ERR;
}

void Socket::set_non_blocking(bool state)
{
#ifndef PLATFORM_WIN
    int flags = fcntl(socket, F_GETFL, 0);
    assert(flags != -1);
    flags = fcntl(socket, F_SETFL, state ? (flags | O_NONBLOCK) : (flags & ~O_NONBLOCK));
    assert(flags != -1);
#endif
}

bool Socket::is_non_blocking() const
{
#ifdef PLATFORM_WIN
    return false;
#else
    int flags = fcntl(socket, F_GETFL, 0);
    assert(flags != -1);
    return flags & O_NONBLOCK;
#endif
}

Status Socket::close()
{
    if (socket == INVALID_SOCKET)
        return SUCCESS;
#ifdef PLATFORM_WIN
    int result = ::closesocket(socket);
    if (result == SOCKET_ERROR)
        result = WSAGetLastError();
#else
    int result = ::close(socket);
#endif
//  socket = INVALID_SOCKET;
    return result == 0 ? SUCCESS : CLOSE_ERR;
}

int Socket::send(const char* buf, int len)
{
    assert(socket != INVALID_SOCKET);
#ifdef PLATFORM_LINUX
    return (int) ::send(socket, buf, len, send_flags);
#else
    return (int) ::send(socket, buf, len, 0);
#endif
}

int Socket::recv(char* buf, int len)
{
    assert(socket != INVALID_SOCKET);
    return (int) ::recv(socket, buf, len, 0);
}

void Socket::find_all_interfaces(Addresses& addresses, int port)
{
    assert(socket != INVALID_SOCKET);
    struct ifaddrs* ifs = 0;
    struct ifaddrs* ifa = 0;
#ifndef PLATFORM_WIN
    getifaddrs(&ifs);
    for (ifa = ifs; ifa; ifa = ifa->ifa_next) {
        if (!ifa->ifa_addr)
            continue;
        if (ifa->ifa_addr->sa_family == AF_INET) {
            // is a valid IP4 Address
            Address* address = Address::create(*ifa->ifa_addr);
            address->set_port(port);
            addresses.append(address);
        } else if (ifa->ifa_addr->sa_family == AF_INET6) {
            // is a valid IP6 Address
            // TODO: tmpAddrPtr = &((struct sockaddr_in6*) ifa->ifa_addr)->sin6_addr;
        }
    }
    if (ifs != 0)
        freeifaddrs(ifs);
#endif
}

//
// class Socket_tcp
//

Socket_tcp::Socket_tcp() : Socket(AF_INET, SOCK_STREAM, IPPROTO_IP)
{
}

void Socket_tcp::init_fd_set(fd_set* set)
{
    FD_SET(socket, set);
}

Status Socket_tcp::select(fd_set* set, unsigned timeout_in_usec)
{
    struct timeval tv;
    tv.tv_sec = timeout_in_usec / 1000000;
    tv.tv_usec = timeout_in_usec % 1000000;
    int sel_res = ::select(FD_SETSIZE, set, NULL, NULL, &tv);
    return sel_res >= 0 ? SUCCESS : SELECT_ERR;
}

Status Socket_tcp::accept(Address* address, Socket_tcp_ref& accepting_socket)
{
    assert(socket != INVALID_SOCKET);
    struct sockaddr addr;
    socklen_t addr_len = sizeof(sockaddr);
    ::memset(&addr, 0, addr_len);
    SOCKET result = ::accept(socket, &addr, &addr_len);
    if (result == INVALID_SOCKET)
        return ACCEPT_ERR;
    accepting_socket = new Socket_tcp(result);
    address->set_addr(addr);
    return SUCCESS;
}

//
// class Socket_tcp_secure_client
//

#if FEATURE_NET_SSL
#ifndef PLATFORM_WIN
#pragma GCC diagnostic push 
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

Socket_tcp_secure_client::Socket_tcp_secure_client() : Socket_tcp()
{
    SSL* ssl = Net_module::module.instance->get_client_ssl_protocol();
    assert(ssl);
    int status = ::SSL_set_fd(ssl, socket);
    assert(status > 0);
    SSL_set_connect_state(ssl);
    SSL_CTX* ctx = Net_module::module.instance->get_ssl_client_ctx();
    SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, NULL);
//  X509* cert = SSL_get_peer_certificate(ssl);
//  char* line = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
//  free(line);
}

Socket_tcp_secure_client::Socket_tcp_secure_client(SOCKET socket) : Socket_tcp(socket)
{
    SSL* ssl = Net_module::module.instance->get_client_ssl_protocol();
    assert(ssl);
    SSL_set_fd(ssl, socket);
}

Socket_tcp_secure_client::~Socket_tcp_secure_client()
{
}

Status Socket_tcp_secure_client::connect(const Address* address)
{
    Status status = Socket_tcp::connect(address);
#ifdef _DEBUG
    if (status != 0) {
        cout << "tcp connect status: " << status << endl;
    }
#endif
    return status;
}

Status Socket_tcp_secure_client::accept(Address* address, Socket_tcp_ref& accepting_socket)
{
    assert(!"TODO: refactor to make this unnecessary");
    assert(socket != INVALID_SOCKET);
    struct sockaddr addr;
    socklen_t addr_len = sizeof(sockaddr);
    ::memset(&addr, 0, addr_len);
    SOCKET client = ::accept(socket, &addr, &addr_len);
    if (client == INVALID_SOCKET)
        return ACCEPT_ERR;
    accepting_socket = new Socket_tcp_secure_client(client);
    address->set_addr(addr);
    SSL* ssl = Net_module::module.instance->get_client_ssl_protocol();
    assert(ssl);
    int status = ::SSL_accept(ssl);
#ifdef _DEBUG
    if (status != 0) {
        char err_buf[256];
        int err = SSL_get_error(ssl, status);
        ERR_error_string_n(err, err_buf, 256);
        cout << "cs accept (" << err << ") " << err_buf << endl;
    }
#endif
//  assert(status == 0);
    return SUCCESS;
}

int Socket_tcp_secure_client::send(const char* buf, int len)
{
    assert(socket != INVALID_SOCKET);
    SSL* ssl = Net_module::module.instance->get_client_ssl_protocol();
    assert(ssl);
    int status = ::SSL_write(ssl, buf, len);
#ifdef _DEBUG
    if (status < 0) {
        char err_buf[256];
        int err = SSL_get_error(ssl, status);
        ERR_error_string_n(err, err_buf, 256);
        cout << "cs send (" << err << ") " << err_buf << endl;
    }
#endif
    return status;
}

int Socket_tcp_secure_client::recv(char* buf, int len)
{
    assert(socket != INVALID_SOCKET);
    SSL* ssl = Net_module::module.instance->get_client_ssl_protocol();
    assert(ssl);
    int status = ::SSL_read(ssl, buf, len);
#ifdef _DEBUG
    if (status < 0) {
        char err_buf[256];
        int err = SSL_get_error(ssl, status);
        ERR_error_string_n(err, err_buf, 256);
        cout << "cs recv (" << err << ") " << err_buf << endl;
    }
#endif
    return status;
}

//
// class Socket_tcp_secure_server
//

Socket_tcp_secure_server::Socket_tcp_secure_server() : Socket_tcp(), ssl(0)
{
    SSL_CTX* ctx = Net_module::module.instance->get_ssl_server_ctx();
    ssl = SSL_new(ctx);
    SSL_set_fd(ssl, socket);
    SSL_set_accept_state(ssl);
    set_non_blocking(false);
}

Socket_tcp_secure_server::Socket_tcp_secure_server(SOCKET socket) : Socket_tcp(socket)
{
    SSL_CTX* ctx = Net_module::module.instance->get_ssl_server_ctx();
    ssl = SSL_new(ctx);
    SSL_set_fd(ssl, socket);
    SSL_set_accept_state(ssl);
    set_non_blocking(false);
}

Socket_tcp_secure_server::~Socket_tcp_secure_server()
{
//  SSL_free(ssl);
}

Status Socket_tcp_secure_server::accept(Address* address, Socket_tcp_ref& accepting_socket)
{
#ifdef _DEBUG
    cout << "ss accept" << endl;
#endif
    assert(socket != INVALID_SOCKET);
    struct sockaddr addr;
    socklen_t addr_len = sizeof(sockaddr);
    ::memset(&addr, 0, addr_len);
    SOCKET client = ::accept(socket, &addr, &addr_len);
    if (client == INVALID_SOCKET)
        return ACCEPT_ERR;
    accepting_socket = new Socket_tcp_secure_server(client);
    address->set_addr(addr);
    assert(ssl);
    int status = ::SSL_accept(ssl);
#ifdef _DEBUG
    if (status != 1) {
        char err_buf[256];
        int err = SSL_get_error(ssl, status);
        ERR_error_string_n(err, err_buf, 256);
        cout << "ss accept: " << status << ": " << err << ": " << err_buf << endl;
    }
#endif
    return SUCCESS;
}

int Socket_tcp_secure_server::send(const char* buf, int len)
{
#ifdef _DEBUG
    cout << "ss send" << endl;
#endif
    assert(socket != INVALID_SOCKET);
    assert(ssl);
    int status;
    int err = 1;
    do {
        status = SSL_write(ssl, buf, len);
        if (status <= 0)
            err = SSL_get_error(ssl, status);
    } while (err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE);
#ifdef _DEBUG
    if (status < 0) {
        char err_buf[256];
        int err = SSL_get_error(ssl, status);
        ERR_error_string_n(err, err_buf, 256);
        cout << "ss send: " << status << ": " << err << ": " << err_buf << endl;
    }
#endif
    return status;
}

int Socket_tcp_secure_server::recv(char* buf, int len)
{
#ifdef _DEBUG
    cout << "ss recv" << endl;
#endif
    assert(socket != INVALID_SOCKET);
    assert(ssl);
    int status;
    int err = 1;
    do {
        status = SSL_read(ssl, buf, len);
        if (status <= 0)
            err = SSL_get_error(ssl, status);
    } while (err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE);
#ifdef _DEBUG
    if (status < 0) {
        char err_buf[256];
        int err = SSL_get_error(ssl, status);
        ERR_error_string_n(err, err_buf, 256);
        cout << "ss recv: " << status << ": " << err << ": " << err_buf << endl;
    }
#endif
    return status;
}

#ifndef PLATFORM_WIN
#pragma GCC diagnostic pop
#endif
#endif

//
// class Socket_udp
//

Socket_udp::Socket_udp() : Socket(AF_INET, SOCK_DGRAM, IPPROTO_IP), max_message_size(0)
{
}

int Socket_udp::get_max_message_size()
{
    assert(socket != INVALID_SOCKET);
    if (max_message_size > 0)
        return max_message_size;
    char* buf = reinterpret_cast<char*>(&max_message_size);
    socklen_t buf_len = 0;
//  int result = ::getsockopt(socket, SOL_SOCKET, SO_MAX_MSG_SIZE, buf, &buf_len);
    int result = ::getsockopt(socket, SOL_SOCKET, SO_SNDBUF, buf, &buf_len);
    return result == 0 ? max_message_size : 0;
}

ssize_t Socket_udp::sendto(const char* buf, int len, Address_const_ref address)
{
    assert(socket != INVALID_SOCKET);
    const struct sockaddr& addr = address->get_addr();
    return ::sendto(socket, buf, len, 0, &addr, sizeof(addr));
}

ssize_t Socket_udp::recvfrom(char* buf, int len, Address_ref& address)
{
    assert(socket != INVALID_SOCKET);
    struct sockaddr addr;
    socklen_t addrlen = sizeof(addr);
    size_t count = ::recvfrom(socket, buf, len, 0, &addr, &addrlen);
    struct sockaddr_in* in_addr = (struct sockaddr_in*) &addr;
    address = new Address_ip4();
    address->set_addr_in(*in_addr);
    return count;
}

//
// class Socket_mcast - TODO: needs work
//

Socket_mcast::Socket_mcast() : Socket_udp()
{
}

Status Socket_mcast::join_group(const Address& group, const Address& source)
{
#ifdef PLATFORM_WIN
    struct ip_mreq_source imr;
    imr.imr_multiaddr = group.get_addr_in().sin_addr;
    imr.imr_sourceaddr = source.get_addr_in().sin_addr;
    imr.imr_interface.S_un.S_addr = INADDR_ANY;
#else
    struct ip_mreq imr;
    imr.imr_multiaddr = group.get_addr_in().sin_addr;
    imr.imr_interface.s_addr = INADDR_ANY;
#endif
    const char* buf = reinterpret_cast<const char*>(&imr);
    socklen_t size = sizeof(imr);
#ifdef PLATFORM_WIN
    int result = ::setsockopt(socket, IPPROTO_IP, IP_ADD_SOURCE_MEMBERSHIP, buf, size);
    if (result == SOCKET_ERROR)
        result = WSAGetLastError();
#else
    int result = ::setsockopt(socket, IPPROTO_IP, IP_ADD_MEMBERSHIP, buf, size);
#endif
    return result == 0 ? SUCCESS : OPTION_ERR;
}

Status Socket_mcast::leave_group(const Address& group, const Address& source)
{
#ifdef PLATFORM_WIN
    struct ip_mreq_source imr;
    imr.imr_multiaddr = group.get_addr_in().sin_addr;
    imr.imr_sourceaddr = source.get_addr_in().sin_addr;
    imr.imr_interface.S_un.S_addr = INADDR_ANY;
#else
    struct ip_mreq imr;
    imr.imr_multiaddr = group.get_addr_in().sin_addr;
    imr.imr_interface.s_addr = INADDR_ANY;
#endif
    const char* buf = reinterpret_cast<const char*>(&imr);
    int size = sizeof(imr);
    int result = ::setsockopt(socket, IPPROTO_IP, IP_DROP_MEMBERSHIP, buf, size);
#ifdef PLATFORM_WIN
    if (result == SOCKET_ERROR)
        result = WSAGetLastError();
#endif
    return result == 0 ? SUCCESS : OPTION_ERR;
}

}}

