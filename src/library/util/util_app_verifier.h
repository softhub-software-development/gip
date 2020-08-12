
//
//  util_app_verifier.h
//
//  Created by Christian Lehner on 11/07/18.
//  Copyright © 2018 softhub. All rights reserved.
//
// https://cboard.cprogramming.com/c-programming/172534-rsa-key-generate-then-encrypt-decrypt-error.html
//

#ifndef LIB_UTIL_APP_VERIFIER_H
#define LIB_UTIL_APP_VERIFIER_H

#include <base/base.h>

namespace SOFTHUB {
namespace UTIL {

class App_verifier : public BASE::Object<> {

public:
    App_verifier();
    ~App_verifier();

    bool verify_self();
};

}}

#endif
