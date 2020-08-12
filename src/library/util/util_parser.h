
//
//  util_parser.h
//
//  Created by Christian Lehner on 11/6/15.
//  Copyright (c) 2019 Softhub. All rights reserved.
//

#ifndef LIB_UTIL_PARSER_H
#define LIB_UTIL_PARSER_H

#include <base/base.h>
#include <net/net.h>
#include "util_cache.h"

namespace SOFTHUB {
namespace UTIL {

FORWARD_CLASS(IDocument_cache);
FORWARD_CLASS(Cached_document);

typedef UTIL::Persistent_cache<NET::Url_const_ref,Cached_document_ref,'shtc'> Url_doc_cache;

class IParser : public BASE::Interface {
};

//
// interfadce IDocument_cache
//

class IDocument_cache : public BASE::Interface {

public:
    virtual Url_doc_cache* get_cache() = 0;
    virtual std::string get_tmp_dir() const = 0;
};

//
// class Cached_document
//

class Cached_document : public BASE::Object<> {

    std::string file_name;

public:
    Cached_document(const std::string& file_name) : file_name(file_name) {}

    const std::string& get_file_name() const { return file_name; }
    bool is_expired() const { return false; } // TOOD
};

}}

#endif
