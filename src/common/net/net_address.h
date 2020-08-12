
//
//  net_address.h
//
//  Created by Christian Lehner on 20/04/16.
//  Copyright (c) 2019 Softhub. All rights reserved.
//

#ifndef NET_ADDRESS_H
#define NET_ADDRESS_H

#include "net_err.h"
#include "net_url.h"
#include <base/base.h>
#include <string>
#ifdef PLATFORM_WIN
#include <winsock2.h>
typedef in_addr in_addr_t;
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif

#define MAC_ADDRESS_SIZE 6

namespace SOFTHUB {
namespace NET {

FORWARD_CLASS(Address);

typedef BASE::Vector<Address_const_ref> Addresses;

//
// class Address
//

class Address : public BASE::Object<> {

    static Status lookup_all_addresses(struct hostent* phe, Addresses& addresses, int port);

public:
    Address() {}

    static Address* create(int port);
    static Address* create(const std::string& ip, int port);
    static Address* create_from_dns_name(const std::string& host, int port);
    static Address* create(struct sockaddr& addr);
    static Status lookup_hostname(std::string& hostname);
    static Status lookup_all_addresses(const std::string& hostname, Addresses& addresses, int port = 0);
    static Status lookup_all_host_addresses(Addresses& addresses, int port = 0);
    static Status lookup_mac_address(const char* iface, unsigned char address[MAC_ADDRESS_SIZE]);

    virtual int get_length() const = 0;
    virtual void set_port(int port) = 0;
    virtual int get_port() const = 0;
    virtual void set_addr(struct sockaddr& addr) = 0;
    virtual const struct sockaddr& get_addr() const = 0;
    virtual void set_addr_in(struct sockaddr_in& addr) = 0;
    virtual const struct sockaddr_in& get_addr_in() const = 0;
    virtual int get_addr_len() const = 0;
    virtual bool is_loopback() const = 0;
    virtual bool is_any() const = 0;
    virtual bool is_private() const = 0;
    virtual Status lookup(std::string& name) = 0;
    virtual bool equals_ignore_port(const Address* address) const = 0;
    virtual const std::string to_string(bool with_port_number = true) const = 0;

    static const Address_const_ref loopback_address;
    static const Address_const_ref any_address;
};

//
// class Address_ip4
//

class Address_ip4 : public Address {

    union {
        struct sockaddr addr;
        struct sockaddr_in addr_in;
    } u;

#ifdef _DEBUG
protected:
    std::string sval;
#endif

public:
    Address_ip4();
    Address_ip4(int port);
    Address_ip4(const char* addr, int port);
    Address_ip4(const struct sockaddr_in& addr, int port);
#ifndef PLATFORM_WIN
    Address_ip4(const struct in_addr& addr, int port);
#endif
    Address_ip4(in_addr_t addr, int port);

    static in_addr_t aton(const char* addr);
    static Address_ip4* create_default(int port);
    static Address_ip4* create_loopback_address(int port);
    static Address_ip4* create_multicast_address(int port);

    int get_length() const { return 4; }
    void set_port(int port) { u.addr_in.sin_port = htons(port); }
    int get_port() const { return ntohs(u.addr_in.sin_port); }
    void set_addr(struct sockaddr& addr);
    const struct sockaddr& get_addr() const { return u.addr; }
    void set_addr_in(struct sockaddr_in& addr);
    const struct sockaddr_in& get_addr_in() const { return u.addr_in; }
    int get_addr_len() const { return sizeof(u.addr_in); }
    bool is_loopback() const { return ntohl(u.addr_in.sin_addr.s_addr) == INADDR_LOOPBACK; }
    bool is_any() const { return ntohl(u.addr_in.sin_addr.s_addr) == INADDR_ANY; }
    bool is_private() const;
    Status lookup(std::string& name);
    void serialize(BASE::Serializer* serializer) const;
    void deserialize(BASE::Deserializer* deserializer);
    bool equals_ignore_port(const Address* address) const;
    bool operator<(const Interface& obj) const;
    bool operator==(const Interface& obj) const;
    size_t hash() const;
    const std::string to_string(bool with_port_number = true) const;

    DECLARE_CLASS('sip4');
};

}}

#endif

