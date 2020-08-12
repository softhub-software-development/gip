
//
//  util_compress.cpp
//
//  Created by Christian Lehner on 12/19/15.
//  Copyright (c) 2019 Softhub. All rights reserved.
//

#include "util_compress.h"
namespace miniz {
#include "util_miniz.h"
}

#undef compress

namespace SOFTHUB {
namespace UTIL {

//
// class Compression
//

int Compression::compress(const byte* src, long src_len, byte*& dst, long& dst_len)
{
    if (dst_len == 0) {
        dst_len = (long) miniz::compressBound(src_len);
        dst = new byte[dst_len];
    }
    return miniz::mz_compress(dst, (unsigned long*) &dst_len, src, (unsigned long) src_len);
}

int Compression::decompress(const byte* src, long src_len, byte*& dst, long& dst_len)
{
    return miniz::mz_uncompress(dst, (unsigned long*) &dst_len, src, (unsigned long) src_len);
}

}}
