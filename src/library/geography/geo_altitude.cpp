
/*
 *  geo_altitude.cpp
 *  NavAid
 *
 *  Created by Christian Lehner on 8/17/10.
 *  Copyright (c) 2019 Softhub. All rights reserved.
 *
 */

#include "stdafx.h"
#include "geo_altitude.h"
#include <util/util_units.h>

namespace SOFTHUB {
namespace GEOGRAPHY {

using namespace SOFTHUB::UTIL;

Geo_altitude::Geo_altitude(Altitude_reference ref, float val) : reference(ref), value(val)
{
}

float Geo_altitude::get_value_in_meters() const
{
    switch (reference) {
    case msl:
    case gnd:
        return (float) Units::feet_to_meters(value);
    case fl:
        return value * 100;
    }
    assert(0);
    return 0;
}

float Geo_altitude::get_value_in_feet() const
{
    switch (reference) {
    case msl:
    case gnd:
        return value;
    case fl:
        return (float) Units::meters_to_feet(value * 100);
    }
    assert(0);
    return 0;
}

Geo_altitude Geo_altitude::parse(const std::string& s)
{
    int result = parse_geo_altitude(s.c_str());
    assert(result == 0);
    Altitude_reference reference = altitude_parse_result.reference;
    float value = altitude_parse_result.value;
    return Geo_altitude(reference, value);
}

}}
