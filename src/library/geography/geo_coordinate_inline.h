
/*
 *  geo_coordinate_inline.h
 *  NavAid
 *
 *  Created by Christian Lehner on 8/11/10.
 *  Copyright (c) 2019 Softhub. All rights reserved.
 *
 */

#ifndef GEOGRAPHY_GEO_COORDINATE_INLINE_H
#define GEOGRAPHY_GEO_COORDINATE_INLINE_H

#include <math.h>

namespace SOFTHUB {
namespace GEOGRAPHY {

template <typename T>
T Geo_coordinate::to_degrees() const
{
    T deg = T(hours) + T(minutes) / 60 + T(seconds) / 3600;
    return deg * sign_of_hemisphere(hemisphere);
}

template <typename T>
T Geo_coordinate::to_radians() const
{
    T deg = to_degrees<T>();
    return deg / 180 * M_PI;
}

template <typename T>
T Geo_coordinate::to_radians(double degree)
{
    return degree / 180 * M_PI;
}

}}

#endif
