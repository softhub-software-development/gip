
/*
 *  geo_position.h
 *  NavAid
 *
 *  Created by Christian Lehner on 8/10/10.
 *  Copyright (c) 2019 Softhub. All rights reserved.
 *
 */

#ifndef GEOGRAPHY_GEO_POSITION_H
#define GEOGRAPHY_GEO_POSITION_H

#include "geo_coordinate.h"

namespace SOFTHUB {
namespace GEOGRAPHY {

//
// Geo_position
//

template <typename T>
class Generic_geo_position {

    T phi;
    T lambda;

public:
    Generic_geo_position(T phi = 0, T lambda = 0) : phi(phi), lambda(lambda) {}

    T get_phi() const { return phi; }
    T get_lambda() const { return lambda; }
    T distance_to(const Generic_geo_position<T>& position) const;
    T heading_to(const Generic_geo_position<T>& position) const;
    Generic_geo_position<T> displace(double heading_in_degree, double nautical_miles);
    Geo_latitude get_latitude() const;
    Geo_longitude get_longitude() const;
    Geo_coordinates get_coordinates() const;
    std::string to_string(Coordinate_format format = standard) const;

    static Generic_geo_position<T> parse(const std::string& coords);
    static Generic_geo_position<T> parse(const std::string& lat, const std::string& lon);
    static const double EARTH_RADIUS_IN_NAUTICAL_MILES;
};

//
// Geo_heading
//

template <typename T>
class Generic_geo_heading {

    T radians;

public:
    Generic_geo_heading(T radians) : radians(radians) {}

    std::string to_string() const;
    std::string to_string_rose() const;

    static std::string to_string_rose(T degree);
};

typedef Generic_geo_position<float> Geo_position;
typedef Generic_geo_heading<float> Geo_heading;

}}

#include "geo_position_inline.h"

#endif
