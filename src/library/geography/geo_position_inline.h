
/*
 *  geo_position_inline.h
 *
 *  Created by Christian Lehner on 8/10/10.
 *  Copyright (c) 2019 Softhub. All rights reserved.
 *
 */

#include "geo_placemark.h"
#include <math.h>
#include <util/util_units.h>

#ifndef GEOGRAPHY_GEO_POSITION_INLINE_H
#define GEOGRAPHY_GEO_POSITION_INLINE_H

namespace SOFTHUB {
namespace GEOGRAPHY {

template <typename T>
const double Generic_geo_position<T>::EARTH_RADIUS_IN_NAUTICAL_MILES = 3440.07;

template <typename T>
Geo_latitude Generic_geo_position<T>::get_latitude() const
{
    return Geo_latitude::from_radians(phi);
}

template <typename T>
Geo_longitude Generic_geo_position<T>::get_longitude() const
{
    return Geo_longitude::from_radians(lambda);
}

template <typename T>
T Generic_geo_position<T>::distance_to(const Generic_geo_position<T>& position) const
{
    assert(fabs(phi) <= 2 * M_PI && fabs(lambda) <= 2 * M_PI);
    assert(fabs(position.phi) <= 2 * M_PI && fabs(position.lambda) <= 2 * M_PI);
    double delta = position.lambda - lambda;
    double sin_phi_s = sin(phi);
    double cos_phi_s = cos(phi);
    double sin_phi_f = sin(position.phi);
    double cos_phi_f = cos(position.phi);
    double sin_delta = sin(delta);
    double cos_delta = cos(delta);
    double a = cos_phi_f * sin_delta;
    double b = cos_phi_s * sin_phi_f - sin_phi_s * cos_phi_f * cos_delta;
    double c = sqrt(a * a + b * b);
    double d = sin_phi_s * sin_phi_f + cos_phi_s * cos_phi_f * cos_delta;
    return EARTH_RADIUS_IN_NAUTICAL_MILES * atan2(c, d);
}

template <typename T>
T Generic_geo_position<T>::heading_to(const Generic_geo_position<T>& position) const
{
//    double delta_phi = position.phi - phi;
    double delta_lambda = position.lambda - lambda;
    double dy = sin(delta_lambda) * cos(position.phi);
    double dx = cos(phi) * sin(position.phi) -
        sin(phi) * cos(position.phi) * cos(delta_lambda);
    double radians = atan2(dy, dx);
    double degree = radians / M_PI * 180;
    return degree >= 0 ? degree : degree + 360;
}

template <typename T>
Generic_geo_position<T> Generic_geo_position<T>::displace(double heading_in_degree, double nautical_miles)
{
    double heading = heading_in_degree / 180 * M_PI;
    double cos_phi = cos(phi);
    if (fabs(cos_phi) <= 1e-5)
        return this;    // TODO: what if we fly over the poles?
    double scale = nautical_miles / (cos_phi * EARTH_RADIUS_IN_NAUTICAL_MILES);
    double delta_phi = scale * cos(heading);
    double delta_lambda = scale * sin(heading);
    return GeoPosition(phi + delta_phi, lambda + delta_lambda);
}

template <typename T>
Generic_geo_position<T> Generic_geo_position<T>::parse(const std::string& s)
{
    Geo_coordinates coordinates = Geo_coordinates::parse(s);
    T phi = coordinates.get_latitude().to_radians<T>();
    T lambda = coordinates.get_longitude().to_radians<T>();
    return Generic_geo_position(phi, lambda);
}

template <typename T>
Generic_geo_position<T> Generic_geo_position<T>::parse(const std::string& lat, const std::string& lon)
{
    Geo_latitude latitude = Geo_latitude::parse(lat);
    Geo_longitude longitude = Geo_longitude::parse(lon);
    T phi = latitude.to_radians<T>();
    T lambda = longitude.to_radians<T>();
    return Generic_geo_position(phi, lambda);
}

template <typename T>
Generic_geo_position<T> Generic_geo_position<T>::create(double lat_in_deg, double lon_in_deg)
{
    T phi = Geo_coordinate::to_radians<T>(lat_in_deg);
    T lambda = Geo_coordinate::to_radians<T>(lon_in_deg);
    return Generic_geo_position(phi, lambda);
}

template <typename T>
std::string Generic_geo_position<T>::to_string(Coordinate_format format) const
{
    return get_latitude().to_string(format) + " " + get_longitude().to_string(format);
}

template <typename T>
std::string Generic_geo_position<T>::find_placemark() const
{
    double lat = get_latitude_in_deg();
    double lon = get_longitude_in_deg();
    return find_placemark(lat, lon);
}

//
// Geo_heading
//

template <typename T>
std::string Generic_geo_heading<T>::to_string() const
{
    T degrees = Geo_coordinate::to_degrees(radians);
    std::stringstream stream;
    stream << degrees;
    return stream.str();
}

template <typename T>
std::string Generic_geo_heading<T>::to_string_rose() const
{
    T deg = Geo_coordinate::to_degrees<T>(radians);
    return to_string_rose(deg);
}

template <typename T>
std::string Generic_geo_heading<T>::to_string_rose(T deg)
{
    std::stringstream stream;
    if (deg > 337.5 || deg < 22.5)
        stream << "N";
    if (22.5 < deg && deg < 67.5)
        stream << "NE";
    if (67.5 < deg && deg < 112.5)
        stream << "E";
    if (112.5 < deg && deg < 157.5)
        stream << "SE";
    if (157.5 < deg && deg < 202.5)
        stream << "S";
    if (202.5 < deg && deg < 247.5)
        stream << "SW";
    if (247.5 < deg && deg < 292.5)
        stream << "W";
    if (292.5 < deg && deg < 337.5)
        stream << "NW";
    return stream.str();
}

}}

#endif
