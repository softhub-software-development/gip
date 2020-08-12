
//
//  net_address.cpp
//
//  Created by Christian Lehner on 20/04/16.
//  Copyright (c) 2019 Softhub. All rights reserved.
//

#include "stdafx.h"
#include "net_address.h"
#ifdef PLATFORM_WIN
#include <Ws2tcpip.h>
#elif defined(PLATFORM_MAC)
#include <sys/socket.h>
#include <sys/sysctl.h>
#include <net/if.h>
#include <net/if_dl.h>
#include <unistd.h>
#else
#include <sys/socket.h>
#include <unistd.h>
#endif
#include <sstream>

using namespace SOFTHUB::BASE;
using namespace std;

namespace SOFTHUB {
namespace NET {

//
// class Address
//

const Address_const_ref Address::loopback_address = Address_ip4::create_loopback_address(0);
const Address_const_ref Address::any_address = new Address_ip4(0);

Address* Address::create(int port)
{
    return new Address_ip4(port);
}

Address* Address::create(const string& ip, int port)
{
    return ip.find('.') ? new Address_ip4(ip.c_str(), port) : 0;
}

Address* Address::create_from_dns_name(const string& host, int port)
{
    int status;
    struct addrinfo hints;
    struct addrinfo* servinfo;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;        // don't care IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM;    // TCP stream sockets
    hints.ai_flags = AI_PASSIVE;        // fill in my IP for me
    Address* address = 0;
    if ((status = getaddrinfo(host.c_str(), 0, &hints, &servinfo)) == 0) {
        if (servinfo->ai_family == AF_INET) {
            struct sockaddr_in* ipv4 = (struct sockaddr_in*) servinfo->ai_addr;
            address = new Address_ip4(ipv4->sin_addr, port);
        } else if (servinfo->ai_family == AF_INET6) {
            struct sockaddr_in6* ipv6 = (struct sockaddr_in6*) servinfo->ai_addr;
            // TODO: IP6
//          address = new Address_ip6(ipv6->sin6_addr, port);
        }
        freeaddrinfo(servinfo);
    } else {
        stringstream stream;
        stream << "getaddrinfo error: " << gai_strerror(status) << " for " << host << endl;
        log_message(INFO, stream.str().c_str());
    }
    return address;
}

Address* Address::create(struct sockaddr& addr)
{
    Address_ip4* address = new Address_ip4();
    address->set_addr(addr);
    return address;
}

Status Address::lookup_hostname(std::string& hostname)
{
    char buf[80];
    if (::gethostname(buf, sizeof(buf)) < 0)
        return NAME_LOOKUP_ERR;
    hostname = buf;
    return SUCCESS;
}

Status Address::lookup_all_addresses(const std::string& hostname, Addresses& addresses, int port)
{
    const char* name = hostname.c_str();
    struct hostent* phe = ::gethostbyname(name);
    if (phe == 0)
        return NAME_LOOKUP_ERR;
    return lookup_all_addresses(phe, addresses, port);
}

Status Address::lookup_all_host_addresses(Addresses& addresses, int port)
{
    char buf[80];
    if (::gethostname(buf, sizeof(buf)) < 0)
        return NAME_LOOKUP_ERR;
    return lookup_all_addresses(buf, addresses, port);
}

Status Address::lookup_all_addresses(struct hostent* phe, Addresses& addresses, int port)
{
    for (int i = 0; phe->h_addr_list[i] != 0; i++) {
        struct in_addr addr;
        memcpy(&addr, phe->h_addr_list[i], sizeof(struct in_addr));
        Address_ip4* address = new Address_ip4(addr, port);
        addresses.append(address);
    }
    return SUCCESS;
}

Status Address::lookup_mac_address(const char* iface, unsigned char address[MAC_ADDRESS_SIZE])
{
#ifdef PLATFORM_MAC
    int mib[6];
    size_t len;
    char* buf;
    unsigned char* ptr;
    struct if_msghdr* ifm;
    struct sockaddr_dl* sdl;
    Status err = MAC_LOOKUP_ERR;

    mib[0] = CTL_NET;
    mib[1] = AF_ROUTE;
    mib[2] = 0;
    mib[3] = AF_LINK;
    mib[4] = NET_RT_IFLIST;
    if ((mib[5] = if_nametoindex(iface)) == 0)
        return err;
    if (sysctl(mib, 6, NULL, &len, NULL, 0) < 0)
        return err;
    if ((buf = (char*) malloc(len)) == NULL)
        return err;
    if (sysctl(mib, 6, buf, &len, NULL, 0) >= 0) {
        ifm = (struct if_msghdr*) buf;
        sdl = (struct sockaddr_dl*) (ifm + 1);
        ptr = (unsigned char*) LLADDR(sdl);
        memcpy(address, ptr, 6);
        err = SUCCESS;
    }
    free(buf);
    return err;
#else
    return INIT_ERR;
#endif
}

//
// class Address_ip4
//

Address_ip4::Address_ip4()
{
    ::memset(&u.addr_in, 0, sizeof(u.addr_in));
}

Address_ip4::Address_ip4(int port)
{
    ::memset(&u.addr_in, 0, sizeof(u.addr_in));
    u.addr_in.sin_family = AF_INET;
    u.addr_in.sin_port = htons(port);
    u.addr_in.sin_addr.s_addr = htonl(INADDR_ANY);
#ifdef PLATFORM_MAC
    u.addr_in.sin_len = sizeof(u.addr_in);
#endif
#ifdef _DEBUG
    sval = to_string();
#endif
}

Address_ip4::Address_ip4(const char* addr, int port)
{
    ::memset(&u.addr_in, 0, sizeof(u.addr_in));
    u.addr_in.sin_family = AF_INET;
    u.addr_in.sin_port = htons(port);
#ifdef PLATFORM_WIN
    ::inet_pton(AF_INET, addr, &u.addr_in.sin_addr);
#else
    ::inet_aton(addr, &u.addr_in.sin_addr);
#endif
#ifdef PLATFORM_MAC
    u.addr_in.sin_len = sizeof(u.addr_in);
#endif
#ifdef _DEBUG
    sval = to_string();
#endif
}

Address_ip4::Address_ip4(const struct sockaddr_in& addr, int port)
{
    ::memset(&u.addr_in, 0, sizeof(u.addr_in));
    u.addr_in.sin_family = addr.sin_family;
    u.addr_in.sin_port = htons(port);
    u.addr_in.sin_addr = addr.sin_addr;
#ifdef _DEBUG
    sval = to_string();
#endif
}

#ifndef PLATFORM_WIN
Address_ip4::Address_ip4(const struct in_addr& addr, int port)
{
    ::memset(&u.addr_in, 0, sizeof(u.addr_in));
    u.addr_in.sin_family = AF_INET;
    u.addr_in.sin_port = htons(port);
    u.addr_in.sin_addr = addr;
#ifdef PLATFORM_MAC
    u.addr_in.sin_len = sizeof(u.addr_in);
#endif
#ifdef _DEBUG
    sval = to_string();
#endif
}
#endif

Address_ip4::Address_ip4(in_addr_t addr, int port)
{
    ::memset(&u.addr_in, 0, sizeof(u.addr_in));
    u.addr_in.sin_family = AF_INET;
    u.addr_in.sin_port = htons(port);
#ifdef PLATFORM_WIN
    u.addr_in.sin_addr.s_addr = addr.S_un.S_addr;
#else
    u.addr_in.sin_addr.s_addr = htonl(addr);
#endif
#ifdef PLATFORM_MAC
    u.addr_in.sin_len = sizeof(u.addr_in);
#endif
#ifdef _DEBUG
    sval = to_string();
#endif
}

void Address_ip4::set_addr(struct sockaddr& addr)
{
    this->u.addr = addr;
#ifdef _DEBUG
    sval = to_string();
#endif
}

void Address_ip4::set_addr_in(struct sockaddr_in& addr)
{
    this->u.addr_in = addr;
#ifdef _DEBUG
    sval = to_string();
#endif
}

bool Address_ip4::is_private() const
{
#ifdef PLATFORM_WIN
    ULONG addr = u.addr_in.sin_addr.S_un.S_addr;
    unsigned long ip = ntohl(addr);
#else
    in_addr_t addr = u.addr_in.sin_addr.s_addr;
    unsigned long ip = ntohl(addr);
#endif
    int a3 = (ip >> 24) & 0xff;
    if (a3 == 10)
        return true;
    int a2 = (ip >> 16) & 0xff;
    if (a3 == 172 && 16 <= a2 && a2 < 32)
        return true;
    if (a3 == 192 &&  a2 == 168)
        return true;
    return false;
}

Status Address_ip4::lookup(string& name)
{
    char host[NI_MAXHOST];
    char serv[NI_MAXSERV];
    int addr_len = get_addr_len();
    int status = getnameinfo(&u.addr, addr_len, host, NI_MAXHOST, serv, NI_MAXSERV, NI_NUMERICSERV);
    if (status != 0)
        return NAME_LOOKUP_ERR;
    name = host;
    return SUCCESS;
}

Address_ip4* Address_ip4::create_default(int port)
{
    return new Address_ip4(port);
}

Address_ip4* Address_ip4::create_loopback_address(int port)
{
#ifdef PLATFORM_WIN
    return new Address_ip4("127.0.0.1", port);  // TODO: review
#else
    return new Address_ip4(INADDR_LOOPBACK, port);
#endif
}

Address_ip4* Address_ip4::create_multicast_address(int port)
{
#ifdef PLATFORM_WIN
    assert(!"TODO");
    return 0;
#else
    return new Address_ip4(INADDR_UNSPEC_GROUP, port);
#endif
}

in_addr_t Address_ip4::aton(const char* addr)
{
#ifdef PLATFORM_WIN
    assert(!"TODO");
#else
    struct in_addr sin_addr;
#ifdef PLATFORM_WIN
    ::inet_pton(AF_INET, addr, &sin_addr);
    return ntohl(sin_addr.S_un.S_addr); // TODO: cast
#else
    ::inet_aton(addr, &sin_addr);
    return ntohl(sin_addr.s_addr);
#endif
#endif
}

void Address_ip4::serialize(Serializer* serializer) const
{
    unsigned int addr = ntohl(u.addr_in.sin_addr.s_addr);
    unsigned short port = ntohs(u.addr_in.sin_port);
    serializer->write(addr);
    serializer->write(port);
}

void Address_ip4::deserialize(Deserializer* deserializer)
{
    unsigned int addr;
    unsigned short port;
    deserializer->read(addr);
    deserializer->read(port);
    u.addr_in.sin_addr.s_addr = htonl(addr);
    u.addr_in.sin_port = htons(port);
#ifdef _DEBUG
    sval = to_string();
#endif
}

bool Address_ip4::equals_ignore_port(const Address* _address) const
{
    const Address_ip4* address = dynamic_cast<const Address_ip4*>(_address);
    return this == address || (address && u.addr_in.sin_addr.s_addr == address->u.addr_in.sin_addr.s_addr);
}

bool Address_ip4::operator<(const Interface& obj) const
{
    const Address* other = dynamic_cast<const Address*>(&obj);
    assert(other);
    return other && ntohl(u.addr_in.sin_addr.s_addr) < ntohl(other->get_addr_in().sin_addr.s_addr);
}

bool Address_ip4::operator==(const Interface& obj) const
{
    const Address* other = dynamic_cast<const Address*>(&obj);
    assert(other);
    return this == other || (other && u.addr_in.sin_addr.s_addr == other->get_addr_in().sin_addr.s_addr && get_port() == other->get_port());
}

size_t Address_ip4::hash() const
{
    struct sockaddr_in addr_in = (struct sockaddr_in&) get_addr();
    return addr_in.sin_addr.s_addr ^ get_port();
}

const std::string Address_ip4::to_string(bool with_port_number) const
{
    char host[INET_ADDRSTRLEN];
    const char* result = inet_ntop(AF_INET, &u.addr_in.sin_addr, host, INET_ADDRSTRLEN);
    assert(result != 0);
//  char* host = ::inet_ntoa(u.addr_in.sin_addr);
    int port = ntohs(u.addr_in.sin_port);
    stringstream stream;
    stream << host;
    if (with_port_number)
        stream << ":" << port;
    return stream.str();
}

}}

