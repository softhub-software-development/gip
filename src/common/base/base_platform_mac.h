
//
//  base_platform_mac.h
//
//  Created by Christian Lehner on 11/8/15.
//  Copyright (c) 2019 Softhub. All rights reserved.
//

#ifndef BASE_PLATFORM_MAC_H
#define BASE_PLATFORM_MAC_H

#define _LIBCPP_HAS_NO_STATIC_ASSERT
#undef static_assert
#define static_assert(...)

#include <type_traits>

#if __MAC_OS_X_VERSION_MAX_ALLOWED < __MAC_10_14
template<> struct __static_assert_test<false> {};
#endif

#define PLATFORM_MAC
#define PLATFORM_UNIX
#define TOOLCHAIN_XCODE

#endif
