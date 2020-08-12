
//
//  base_container.h
//
//  Created by Softhub.
//  Copyright (c) 2019 Softhub. All rights reserved.
//

#ifndef BASE_CONTAINER_H
#define BASE_CONTAINER_H

#include "base_reference.h"
#include "base_stl_wrapper.h"

namespace SOFTHUB {
namespace BASE {

//
// interface IContainer
//

class IContainer : public Serializable {

public:
    virtual ~IContainer() {}

    virtual size_t get_size() const = 0;
};

}}

#include "base_container_inline.h"

#endif
