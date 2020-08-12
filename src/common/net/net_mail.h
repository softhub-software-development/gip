
//
//  net_mail.h
//
//  Created by Christian Lehner on 07/15/19.
//  Copyright (c) 2019 Softhub. All rights reserved.
//

#ifndef NET_MAIL_H
#define NET_MAIL_H

#include <base/base.h>

namespace SOFTHUB {
namespace NET {

FORWARD_CLASS(Mail_sender);

//
// class Mail_sender
//

class Mail_sender : public BASE::Object<> {

    std::string sender;
    std::string mail_host;

    static std::string date_time_message_id();
    static std::string date_formatted();

public:
    Mail_sender(const std::string& sender) : sender(sender) {}

    void set_sender(const std::string& sender) { this->sender = sender; }
    const std::string& get_sender() const { return sender; }
    void set_mail_host(const std::string& mail_host) { this->mail_host = mail_host; }
    const std::string& get_mail_host() const { return mail_host; }
    int send(const std::string& to, const std::string& subject, const std::string& body);
};

}}

#endif
