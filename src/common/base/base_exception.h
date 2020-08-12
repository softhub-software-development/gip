
//
//  base_exception.h
//
//  Created by Christian Lehner on 6/26/12.
//  Copyright (c) 2019 Softhub. All rights reserved.
//

#ifndef BASE_EXCEPTION_H
#define BASE_EXCEPTION_H

#include <stdexcept>
#include <string>

namespace SOFTHUB {
namespace BASE {

//
// class Exception
//

class Exception : public std::runtime_error {

public:
    Exception(const std::string& msg) : runtime_error(msg) {}

    virtual std::string get_message() const { return what(); }
};

}}

#endif
