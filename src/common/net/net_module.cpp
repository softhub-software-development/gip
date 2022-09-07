
//
//  net_module.cpp
//
//  Created by Christian Lehner on 8/15/12.
//  Copyright (c) 2019 Softhub. All rights reserved.
//

#include "stdafx.h"
#include "net.h"
#ifdef PLATFORM_WIN
#include <Ws2tcpip.h>
#endif

#define TEST_ADDRESSES 0
#define TEST_CONNECTIONS 0

#ifdef PLATFORM_WIN
#pragma comment(lib, "crypt32")
#define SSL_DEFAULT_CERT_PEM "..\\..\\certificates\\cert1.pem"
#define SSL_DEFAULT_PRIV_KEY "..\\..\\certificates\\privkey1.pem"
#define SSL_DEFAULT_FULL_CHAIN "..\\..\\certificates\\fullchain1.pem"
#elif defined PLATFORM_MAC
//#define SSL_APPLE_PAY_CERT_PEM "../../certificates/apple_pay_sandbox.pem"
#define SSL_DEFAULT_CERT_PEM "../../certificates/cert1.pem"
#define SSL_DEFAULT_PRIV_KEY "../../certificates/privkey1.pem"
#define SSL_DEFAULT_FULL_CHAIN "../../certificates/fullchain1.pem"
#else
#define SSL_DEFAULT_CERT_PEM "/etc/letsencrypt/live/softhub.com/cert.pem"
#define SSL_DEFAULT_PRIV_KEY "/etc/letsencrypt/live/softhub.com/privkey.pem"
#define SSL_DEFAULT_FULL_CHAIN "/etc/letsencrypt/live/softhub.com/fullchain.pem"
#endif

using namespace SOFTHUB::BASE;
using namespace SOFTHUB::HAL;
using namespace std;

namespace SOFTHUB {
namespace NET {

BASE::Module<Net_module> Net_module::module;

typedef union {
    unsigned char native[MAC_ADDRESS_SIZE];
    ularge external;
} Net_mac_address;

static Net_mac_address mac_address;

#ifdef __GNUC__
#pragma GCC diagnostic push 
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

Net_module::Net_module()
{
    Hal_module::module.init();
    Base_module::register_class<Address_ip4>();
    Base_module::register_class<Url>();
#ifdef PLATFORM_WIN
    status = WSAStartup(MAKEWORD(2, 0), &wsa_data) == 0 ? SUCCESS : INIT_ERR;
#endif
#if FEATURE_NET_SSL
//  ERR_load_crypto_strings();
//  SSL_load_error_strings();
//  SSL_library_init();
//  OpenSSL_add_all_algorithms();
    init_client_context();
    init_server_context();
    client_ssl = SSL_new(client_ctx);
    server_ssl = SSL_new(server_ctx);
#endif
    Status mac_addr_status = Address::lookup_mac_address("en0", mac_address.native);
#ifdef PLATFORM_MAC
    assert(mac_addr_status == SUCCESS);
#endif
}

Net_module::~Net_module()
{
    Base_module::unregister_class<Address_ip4>();
    Base_module::unregister_class<Url>();
    Hal_module::module.dispose();
#if FEATURE_NET_SSL
    SSL_shutdown(client_ssl);
    SSL_free(client_ssl);
    SSL_shutdown(server_ssl);
    SSL_free(server_ssl);
//  ERR_free_strings();
    SSL_CTX_free(client_ctx);
    SSL_CTX_free(server_ctx);
//  EVP_cleanup();
#endif
#ifdef PLATFORM_WIN
    WSACleanup();
#endif
}

#if FEATURE_NET_SSL

void Net_module::init_context(SSL_CTX* ctx)
{
    SSL_CTX_set_options(ctx, SSL_OP_SINGLE_DH_USE);
    int use_cert = SSL_CTX_use_certificate_file(ctx, SSL_DEFAULT_CERT_PEM, SSL_FILETYPE_PEM);
    if (use_cert <= 0)
        ERR_print_errors_fp(stdout);
    int use_priv = SSL_CTX_use_PrivateKey_file(ctx, SSL_DEFAULT_PRIV_KEY, SSL_FILETYPE_PEM);
    if (use_priv <= 0)
        ERR_print_errors_fp(stdout);
    int use_chain = SSL_CTX_use_certificate_chain_file(ctx, SSL_DEFAULT_FULL_CHAIN);
    if (use_chain <= 0)
        ERR_print_errors_fp(stdout);
#ifdef _DEBUG
    cout << "use_cert: " << use_cert << " use_priv: " << use_priv << " use_chain: " << use_chain << endl;
#endif
}

void Net_module::init_client_context()
{
    const SSL_METHOD* method = SSLv23_client_method();
    client_ctx = SSL_CTX_new(method);
    init_context(client_ctx);
}

void Net_module::init_server_context()
{
    const SSL_METHOD* method = SSLv23_server_method();
    server_ctx = SSL_CTX_new(method);
    init_context(server_ctx);
}

#endif

ularge Net_module::get_mac_address()
{
    return mac_address.external;
}

void Net_module::configure(Configuration* config)
{
#if FEATURE_NET_SSL
#endif
}

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

#ifdef _DEBUG

static void test_addresses()
{
    Address_const_ref a1 = Address::create("192.168.0.103", 1963);
    Address_const_ref a2 = Address::create("192.168.0.107", 1963);
    Address_const_ref a3 = Address::create("192.168.0.107", 1963);
    Address_const_ref a4 = Address::create_from_dns_name("pi", 1963);
    Address_const_ref a5 = Address::create("172.16.0.1", 1963);
    Address_const_ref a6 = Address::create("10.10.10.10", 1963);
    Address_const_ref a7 = Address::create("110.0.20.47", 1963);
    Address_const_ref a8 = Address::create("172.32.0.1", 1963);
    assert(a1 != a2);
    assert(a2 == a3);
    assert(a1 == a4);
    assert(a1->is_private());
    assert(a5->is_private());
    assert(a6->is_private());
    assert(!a7->is_private());
    assert(!a8->is_private());
}

static void test_urls()
{
    string s = "a;92fejn nas9+e=-9o";
    string e = Url::encode(s);
    string d = Url::decode(e);
    assert(s == d);
#ifndef PLATFORM_WIN
    string dash = "–";
    string enc_dash = Url::encode(dash);
    string dec_dash = Url::decode(enc_dash);
    assert(dash == dec_dash);
#endif
    Url_ref url = Url::create("http://www.softhub.com:80/?x=1&y=2&z");
    Url_parameters parameters;
    url->parse_parameters(parameters);
    assert(parameters.size() == 3);
    assert(parameters[0].first == "x");
    assert(parameters[0].second == "1");
    assert(parameters[1].first == "y");
    assert(parameters[1].second == "2");
    assert(parameters[2].first == "z");
    assert(parameters[2].second == "");
    Url_ref url2 = Url::create("http://www.softhub.com?s");
    Url_parameters parameters2;
    url2->parse_parameters(parameters2);
    assert(parameters2.size() == 1);
    Url_ref url3 = Url::create("");
    assert(url3);
    string host1, dom1, tld1;
    Url::parse_server_name("www.softhub.com", host1, dom1, tld1);
    assert(host1 == "www" && dom1 == "softhub" && tld1 == "com");
    string host2, dom2, tld2;
    Url::parse_server_name("softhub.de", host2, dom2, tld2);
    assert(host2 == "" && dom2 == "softhub" && tld2 == "de");
    string host3, dom3, tld3;
    Url::parse_server_name("m.hub.softhub.com", host3, dom3, tld3);
    assert(host3 == "m.hub" && dom3 == "softhub" && tld3 == "com");
    Address_ref ip0 = Address::create("91.64.54.207", 0);
    Address_ref ip1 = Address::create("91.65.54.207", 0);
    assert(ip0 < ip1);
    Address_ref ip2 = Address::create("20.65.54.207", 0);
    assert(ip2 < ip0);
    Url_ref u0 = Url::create("http://www.softhub.com");
    Url_ref u1 = Url::create("http://www.softhub.com/");
    Url_ref u2 = Url::create("http://www.softhub.com/a/");
    Url_ref u3 = Url::create("http://www.softhub.com/a");
    Url_ref u4 = Url::create("http://www.softhub.com/b");
    Url_ref u5 = Url::create("http://www.softhub.com/b//");
    Url_ref u6 = Url::create("http://www.softhub.com/bbb/ccc");
    Url_ref u7 = Url::create("http://www.softhub.com/bbb/ccc/");
    Url_ref u8 = Url::create("http://www/bbb/ccc/");
    Url_ref u9 = Url::create("http://softhub.com/bbb/ccc/");
    assert(u0->is_same_path(u1));
    assert(!u1->is_same_path(u2));
    assert(u2->is_same_path(u3));
    assert(!u2->is_same_path(u4));
    assert(u4->is_same_path(u5));
    assert(u6->is_same_path(u7));
    assert(!u5->is_same_path(u6));
    assert(u0->norm_path() == u0->get_path());
    assert(u1->norm_path() == u0->get_path());
    assert(u5->norm_path() == u4->get_path());
    assert(u0->top_level_domain() == "com");
    assert(u0->domain_name() == "www.softhub.com");
    assert(u8->top_level_domain() == "");
    assert(u8->domain_name() == "www");
    assert(u9->top_level_domain() == "com");
}

static void test_connection1(Http_factory* factory)
{
    cout << "test_connection1" << endl;
    Url_ref url = Url::create("http://www.softhub.com/wp0/wp-login.php");
    Http_connection_ref connection = factory->create_connection();
    Http_request_ref request(new Http_request(get_method, url));
    request->set_caching(true);
    Http_response_ref response = connection->query(request);
}

static void test_connection2(Http_factory* factory)
{
    cout << "test_connection2" << endl;
    Url_ref url = Url::create("http://www.softhub.com");
    Http_connection_ref connection = factory->create_connection();
    Http_request_ref request(new Http_request(get_method, url));
    request->set_caching(true);
    Http_response_ref response = connection->query(request);
}

static void test_ssl_connection(Http_factory* factory)
{
    cout << "test_ssl_connection" << endl;
    Url_ref url = Url::create("https://www.softhub.com");
    Http_connection_ref connection = factory->create_connection();
    Http_request_ref request(new Http_request(get_method, url));
    cout << "first ssl access" << endl;
    Http_response_ref response1 = connection->query(request);
    assert(response1 && response1->get_header());
    cout << "second ssl access" << endl;
    Http_response_ref response2 = connection->query(request);
    assert(response2 && response2->get_header());
    Http_request_ref request3(new Http_request(post_method, url));
    Http_response_ref response3 = connection->query(request3);
    assert(response3 && response3->get_header());
    cout << "post ssl access" << endl;
    Url_ref url2 = Url::create("https://api.sandbox.paypal.com");
    Http_request_ref request4(new Http_request(post_method, url2));
    Http_response_ref response4 = connection->query(request4);
    assert(response4 && response4->get_header());
    cout << "post ssl access to paypal" << endl;
    Url_ref url5 = Url::create("https://biz.softhub.com:8081");
    Http_request_ref request5(new Http_request(post_method, url5));
    Http_connection_ref connection5 = factory->create_connection();
    Http_response_ref response5 = connection5->query(request5);
    cout << "ssl access on port 8081" << endl;
}

#ifdef NETWORK_OBSERVER_SUPPORT
static void test_network_observer()
{
    Net_observer_ref observer(new Net_observer());
    observer->test();
}
#endif

void Net_module::test()
{
#if 0
    String_vector addresses;
    Status status = Address::lookup_hostaddr(addresses);
    assert(status == SUCCESS);
    Url_ref u1 = Url::create("http://www.softhub.com:8080/login");
    assert(u1 && u1->get_protocol() == "http" && u1->get_host() == "www.softhub.com" && u1->get_port() == 8080 && u1->get_path() == "/login");
    Url_ref u2 = Url::create("www.softhub.com");
    assert(u2 && u2->get_protocol() == "http" && u2->get_host() == "www.softhub.com" && u2->get_port() == 80 && u2->get_path() == "");
    Url_ref u3 = Url::create("http://www.softhub.com:8080?login.jsp");
    assert(u3 && u3->get_protocol() == "http" && u3->get_host() == "www.softhub.com" && u3->get_port() == 8080 && u3->get_path() == "?login.jsp");
    Url_ref u4 = Url::create("http:///index.html");
    assert(u4 && u4->get_protocol() == "http" && u4->get_host() == "localhost" && u4->get_port() == 80 && u4->get_path() == "/index.html");
    Url_ref u5 = Url::create("www.softhub.com/index.html");
    assert(u5 && u5->get_protocol() == "http" && u5->get_host() == "www.softhub.com" && u5->get_port() == 80 && u5->get_path() == "/index.html");
    Url_ref u6 = Url::create("ftp://www.softhub.com/index.html");
    assert(u6 && u6->get_protocol() == "ftp" && u6->get_host() == "www.softhub.com" && u6->get_port() == 80 && u6->get_path() == "/index.html");
    Url_ref u7 = Url::create("http://www.ftd.de/politik/:berliner");
    assert(u7 && u7->get_protocol() == "http" && u7->get_host() == "www.ftd.de" && u7->get_port() == 80 && u7->get_path() == "/politik/:berliner");
#endif
    test_urls();
#if TEST_ADDRESSES
    test_addresses();
#endif
#if TEST_CONNECTIONS
    Http_factory_ref factory(new Http_factory());
    test_connection1(factory);
    test_connection2(factory);
    test_ssl_connection(factory);
#endif
#ifdef PLATFORM_MAC
    unsigned char a[MAC_ADDRESS_SIZE];
    Status result = Address::lookup_mac_address("en0", a);
    assert(result == SUCCESS);
    printf("%02x:%02x:%02x:%02x:%02x:%02x\n", a[0], a[1], a[2], a[3], a[4], a[5]);
#endif
#ifdef NETWORK_OBSERVER_SUPPORT
    test_network_observer();
#endif
}

#endif

}}
