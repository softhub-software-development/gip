
//
//  util_compress.h
//
//  Created by Christian Lehner on 12/19/15.
//  Copyright (c) 2019 Softhub. All rights reserved.
//

#ifndef LIB_UTIL_COMPRESS_H
#define LIB_UTIL_COMPRESS_H

#include <base/base.h>

namespace SOFTHUB {
namespace UTIL {

//
// class Compression
//

class Compression {

public:
    static int compress(const byte* src, long src_len, byte*& dst, long& dst_len);
    static int decompress(const byte* src, long src_len, byte*& dst, long& dst_len);
};

}}

#endif
