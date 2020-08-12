
//
//  net_err.h
//
//  Created by Christian Lehner on 20/04/16.
//  Copyright (c) 2019 Softhub. All rights reserved.
//

#ifndef NET_NET_ERR_H
#define NET_NET_ERR_H

namespace SOFTHUB {
namespace NET {

enum Status {
    SUCCESS = 0,
    INIT_ERR = 1,
    ACCEPT_ERR = 2,
    BIND_ERR = 3,
    LISTEN_ERR = 4,
    SELECT_ERR = 5,
    CONNECT_ERR = 6,
    SEND_ERR = 7,
    CLOSE_ERR = 8,
    OPTION_ERR = 9,
    NAME_LOOKUP_ERR = 10,
    MAC_LOOKUP_ERR = 11
};

}}

#endif

