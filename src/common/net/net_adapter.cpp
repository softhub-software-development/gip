
//
//  net_adapter.cpp
//
//  Created by Christian Lehner on 20/04/16.
//  Copyright (c) 2019 Softhub. All rights reserved.
//

#include "stdafx.h"
#include "net_adapter.h"
#include "net_socket.h"
#include "net_address.h"
#ifdef PLATFORM_WIN
#include <Ws2tcpip.h>
#endif

namespace SOFTHUB {
namespace NET {

using namespace BASE;
using namespace HAL;

Net_adapter::Net_adapter()
{
//  assert(Net_module::module is initialized);  // TODO
    socket = new Socket_mcast();
    group_address = Address_ip4::create_multicast_address(22430);
    local_address = Address_ip4::create_loopback_address(22430);
    status = local_address->lookup(hostname);
    if (status != SUCCESS)
        return;
    status = socket->connect(local_address);
    if (status != SUCCESS)
        return;
    status = socket->join_group(*group_address, *local_address);
    if (status != SUCCESS)
        log_message(WARN, "cannot join multicast group");
#ifdef DEBUG_MONKEY
    init_remote_mockup();
#endif
}

Net_adapter::~Net_adapter()
{
#ifdef DEBUG_MONKEY
    dispose_remote_mockup();
#endif
    status = socket->leave_group(*group_address, *local_address);
    if (status != SUCCESS)
        log_message(WARN, "cannot leave multicast group");
    delete local_address;
    delete group_address;
}

#ifdef DEBUG_MONKEY
void Net::init_remote_mockup()
{
    monkey = new Thread(new Monkey(local_address));
    monkey->start();
    Thread::sleep(1000);

    char buf[10];
    buf[0] = 'A';
    buf[1] = 'B';
    buf[2] = 'C';
    buf[3] = 'D';
    buf[4] = 'E';
    buf[5] = 'F';
    buf[6] = 'G';
    buf[7] = 'H';
    buf[8] = 'I';
    buf[9] = 'J';
    int count = socket->send(buf, 10);
#ifdef PLATFORM_WIN
    int werr = WSAGetLastError();
#endif
    buf[0] = 'Z';
    count = socket->send(buf, 1);
    delete thread;
}

void Net::dispose_remote_mockup()
{
    monkey->stop();
    delete monkey;
}
#endif

Packet::Packet()
{
}

#ifdef _DEBUG

Monkey::Monkey(Address* address) : local_address(address)
{
}

Monkey::~Monkey()
{
}

void Monkey::run()
{
    Socket_mcast* socket = new Socket_mcast();
    Status status = socket->bind(local_address);
    assert(status == SUCCESS);
    while (true) {
        char buf[11];
        int count = socket->recv(buf, 10);
        buf[10] = 0;
        log_message(INFO, buf);
        count = 0;    // compiler warning
    }
}

#endif

}}

