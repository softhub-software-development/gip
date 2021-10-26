
//
//  net_socket.h
//
//  Created by Christian Lehner on 20/04/16.
//  Copyright (c) 2019 Softhub. All rights reserved.
//

#ifndef NET_SOCKET_H
#define NET_SOCKET_H

#include "net_address.h"
#include "net_err.h"
#include <base/base.h>
#ifdef PLATFORM_WIN
#include <winsock2.h>
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#else
#include <sys/socket.h>
#include <unistd.h>
#define SOCKET int
#endif
#if FEATURE_NET_SSL
#include <openssl/ssl.h>
#include <openssl/err.h>
#endif

namespace SOFTHUB {
namespace NET {

FORWARD_CLASS(Socket);
FORWARD_CLASS(Socket_tcp);
FORWARD_CLASS(Socket_tcp_secure_client);
FORWARD_CLASS(Socket_tcp_secure_server);
FORWARD_CLASS(Socket_udp);

//
// class Socket
//

class Socket : public BASE::Object<> {

protected:
    SOCKET socket;
#ifdef PLATFORM_LINUX
    int send_flags;
#elif defined PLATFORM_WIN
	bool non_blocking;
#endif

    Socket(int family, int type, int protocol);
    Socket(SOCKET socket);

public:
    Socket();
    virtual ~Socket();

    SOCKET get_socket_fd() const { return socket; }
    void set_non_blocking(bool state);
    bool is_non_blocking() const;
    bool report_error(int* val) const;

    virtual Status bind(const Address* address);
    virtual Status listen(int back_log = SOMAXCONN);
    virtual Status connect(const Address* address);
    virtual Status no_sig_pipe(int state);
    virtual Status reusable(bool state);
    virtual Status set_send_timeout(int msec);
    virtual Status set_recieve_timeout(int msec);
    virtual Status close();
    virtual int send(const char* buf, int len);
    virtual int recv(char* buf, int len);

    void find_all_interfaces(Addresses& addresses, int port);

    static void report_last_error();
};

//
// class Socket_tcp
//

class Socket_tcp : public Socket {

public:
    Socket_tcp();
    Socket_tcp(SOCKET socket) : Socket(socket) {}

    virtual Status select(fd_set* fds, unsigned timeout_in_usec = 0);
    virtual Status accept(Address* address, Socket_tcp_ref& accepting_socket);
};

//
// class Socket_tcp_secure_client
//

class Socket_tcp_secure_client : public Socket_tcp {

protected:
    Socket_tcp_secure_client(SOCKET socket);

public:
    Socket_tcp_secure_client();
    ~Socket_tcp_secure_client();

    Status connect(const Address* address);
    Status accept(Address* address, Socket_tcp_ref& accepting_socket);
    int send(const char* buf, int len);
    int recv(char* buf, int len);
};

//
// class Socket_tcp_secure_server
//

class Socket_tcp_secure_server : public Socket_tcp {

#if FEATURE_NET_SSL
    SSL* ssl;
#endif

protected:
    Socket_tcp_secure_server(SOCKET socket);

public:
    Socket_tcp_secure_server();
    ~Socket_tcp_secure_server();

    Status accept(Address* address, Socket_tcp_ref& accepting_socket);
    int send(const char* buf, int len);
    int recv(char* buf, int len);
};

//
// class Socket_udp
//

class Socket_udp : public Socket {

    int max_message_size;

protected:
    Socket_udp(SOCKET socket) : Socket(socket) {}

public:
    Socket_udp();

    int get_max_message_size();
    ssize_t sendto(const char* buf, int len, Address_const_ref address);
    ssize_t recvfrom(char* buf, int len, Address_ref& address);
};

//
// class Socket_mcast - TODO: out of date, needs work
//

class Socket_mcast : public Socket_udp {

protected:
    Socket_mcast(SOCKET socket) : Socket_udp(socket) {}

public:
    Socket_mcast();

    Status join_group(const Address& group, const Address& source);
    Status leave_group(const Address& group, const Address& source);
};

}}

#endif

