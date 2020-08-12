
//
//  net_module.h
//
//  Created by Christian Lehner on 8/15/12.
//  Copyright (c) 2019 Softhub. All rights reserved.
//

#ifndef NET_NET_MODULE_H
#define NET_NET_MODULE_H

#include "net_err.h"
#include <hal/hal.h>
#ifdef PLATFORM_WIN
#include <winsock2.h>
#else
#if FEATURE_NET_SSL
#include <openssl/ssl.h>
#include <openssl/err.h>
#endif
#include <sys/socket.h>
#include <sys/time.h>
#endif

namespace SOFTHUB {
namespace NET {

class Net_module : public BASE::Object<> {

#ifdef PLATFORM_WIN
    WSADATA wsa_data;
#endif
    Status status;
#if FEATURE_NET_SSL
    HAL::Mutex mutex;
    SSL_CTX* client_ctx;
    SSL_CTX* server_ctx;
    SSL* client_ssl;
    SSL* server_ssl;

    void init_context(SSL_CTX* ctx);
    void init_client_context();
    void init_server_context();
#endif

public:
    Net_module();
    ~Net_module();

    Status get_status() const { return status; }

#if FEATURE_NET_SSL
    SSL_CTX* get_ssl_client_ctx() { return client_ctx; }
    SSL_CTX* get_ssl_server_ctx() { return server_ctx; }
    SSL* get_client_ssl_protocol() { return client_ssl; }
    SSL* get_server_ssl_protocol() { return server_ssl; }
#endif

    void configure(BASE::Configuration* config);

    static BASE::Module<Net_module> module;
    static ularge get_mac_address();
#ifdef _DEBUG
    static void test();
#endif
};

}}

#endif

