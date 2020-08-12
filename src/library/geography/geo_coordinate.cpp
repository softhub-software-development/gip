
/*
 *  geo_coordinate.cpp
 *
 *  Created by Christian Lehner on 8/9/10.
 *  Copyright (c) 2019 Softhub. All rights reserved.
 *
 */

#include "stdafx.h"
#include "geo_coordinate.h"
#include "geo_parse_types.h"
#include <assert.h>
#include <math.h>
#include <sstream>
#include <iomanip>

using namespace SOFTHUB::BASE;
using namespace std;

namespace SOFTHUB {
namespace GEOGRAPHY {

char Geo_coordinate::short_hemisphere(Hemisphere hemisphere)
{
    switch (hemisphere) {
    case north:
        return 'N';
    case east:
        return 'E';
    case south:
        return 'S';
    case west:
        return 'W';
    default:
        assert(0);
        return '\0';
    }
}

int Geo_coordinate::sign_of_hemisphere(Hemisphere hemisphere)
{
    switch (hemisphere) {
    case north:
        return 1;
    case east:
        return 1;
    case south:
        return -1;
    case west:
        return -1;
    default:
        assert(0);
        return 1;
    }
}

Hemisphere Geo_coordinate::hemisphere_from(char c)
{
    switch (c) {
    case 'N':
        return north;
    case 'E':
        return east;
    case 'S':
        return south;
    case 'W':
        return west;
    default:
        assert(0);
        return unspecified;
    }
}

string Geo_coordinate::coordinate_format(Coordinate_format format) const
{
    stringstream stream;
    switch (format) {
    default:
    case standard:
        stream << setfill('0') << setw(2) << hours << " "
               << setfill('0') << setw(2) << minutes << " "
               << setfill('0') << setw(2) << setiosflags(ios::fixed) << setprecision(3) << seconds << " "
               << short_hemisphere(hemisphere);
        break;
    case with_units:
        stream << setfill('0') << setw(2) << hours << "° "
               << setfill('0') << setw(2) << minutes << "' "
               << setfill('0') << setw(2) << setiosflags(ios::fixed) << setprecision(3) << seconds << "\" "
               << short_hemisphere(hemisphere);
        break;
    case decimal:
        stream << to_degrees<double>();
        break;
    }
    return stream.str();
}

void Geo_coordinate::serialize(BASE::Serializer* serializer) const
{
    serializer->write(hours);
    serializer->write(minutes);
    serializer->write(seconds);
    unsigned char tmp = (unsigned char) hemisphere;
    serializer->write(tmp);
}

void Geo_coordinate::deserialize(BASE::Deserializer* deserializer)
{
    deserializer->read(hours);
    deserializer->read(minutes);
    deserializer->read(seconds);
    unsigned char tmp;
    deserializer->read(tmp);
    hemisphere = (Hemisphere) tmp;
}

bool Geo_coordinate::operator==(const Geo_coordinate& obj) const
{
    return hours == obj.hours && minutes == obj.minutes && seconds == obj.seconds && hemisphere == obj.hemisphere;
}

size_t Geo_coordinate::hash() const
{
    return hours * minutes + seconds * hemisphere;
}

//
// Geo_latitude
//

Geo_latitude Geo_latitude::operator+(double r) const
{
    return Geo_latitude(hours, minutes + r, seconds, hemisphere); // TODO: r in degree
}

Geo_latitude Geo_latitude::operator-(double r) const
{
    return Geo_latitude(hours, minutes - r, seconds, hemisphere); // TODO: r in degree
}

string Geo_latitude::to_string(Coordinate_format format) const
{
    return coordinate_format(format);
}

Geo_latitude Geo_latitude::from_radians(double phi)
{
    double degree = phi * 180 / M_PI;
    double abs_degree = fabs(degree);
    int h = (int) floor(abs_degree);
    double minutes = (abs_degree - h) * 60;
    int m = (int) floor(minutes);
    double s = (minutes - m) * 60;
    Hemisphere hemi = phi >= 0 ? north : south;
    return Geo_latitude(h, m, s, hemi);
}

#ifndef NO_GEO_PARSER
Geo_latitude Geo_latitude::parse(const string& s)
{
    int result = parse_geo_latitude(s.c_str());
    assert(result == 0);
    int hours = coord_parse_result[0].hours;
    int minutes = coord_parse_result[0].minutes;
    float seconds = coord_parse_result[0].seconds;
    Hemisphere hemisphere = Geo_coordinate::hemisphere_from(coord_parse_result[0].hemisphere);
    return Geo_latitude(hours, minutes, seconds, hemisphere);
}
#endif

Geo_longitude Geo_longitude::operator+(double r) const
{
    return Geo_longitude(hours, minutes + r, seconds, hemisphere); // TODO: r in degree
}

Geo_longitude Geo_longitude::operator-(double r) const
{
    return Geo_longitude(hours, minutes - r, seconds, hemisphere); // TODO: r in degree
}

string Geo_longitude::to_string(Coordinate_format format) const
{
    return coordinate_format(format);
}

Geo_longitude Geo_longitude::from_radians(double lambda)
{
    double degree = lambda * 180 / M_PI;
    double abs_degree = fabs(degree);
    int h = (int) floor(abs_degree);
    double minutes = (abs_degree - h) * 60;
    int m = (int) floor(minutes);
    double s = (minutes - m) * 60;
    Hemisphere hemi = lambda >= 0 ? east : west;
    return Geo_longitude(h, m, s, hemi);
}

#ifndef NO_GEO_PARSER
Geo_longitude Geo_longitude::parse(const string& s)
{
    int result = parse_geo_longitude(s.c_str());
    assert(result == 0);
    int hours = coord_parse_result[1].hours;
    int minutes = coord_parse_result[1].minutes;
    float seconds = coord_parse_result[1].seconds;
    Hemisphere hemisphere = Geo_coordinate::hemisphere_from(coord_parse_result[1].hemisphere);
    return Geo_longitude(hours, minutes, seconds, hemisphere);
}
#endif

//
// Geo_coordinates
//

#ifndef NO_GEO_PARSER
Geo_coordinates Geo_coordinates::parse(const string& s)
{
    int result = parse_geo_coordinate(s.c_str());
    assert(result == 0);
    int lat_hours = coord_parse_result[0].hours;
    int lat_minutes = coord_parse_result[0].minutes;
    float lat_seconds = coord_parse_result[0].seconds;
    Hemisphere lat_hemisphere = Geo_coordinate::hemisphere_from(coord_parse_result[0].hemisphere);
    Geo_latitude latitude(lat_hours, lat_minutes, lat_seconds, lat_hemisphere);
    int lon_hours = coord_parse_result[1].hours;
    int lon_minutes = coord_parse_result[1].minutes;
    float lon_seconds = coord_parse_result[1].seconds;
    Hemisphere lon_hemisphere = Geo_coordinate::hemisphere_from(coord_parse_result[1].hemisphere);
    Geo_longitude longitude(lon_hours, lon_minutes, lon_seconds, lon_hemisphere);
    return Geo_coordinates(latitude, longitude);
}
#endif

void Geo_coordinates::serialize(BASE::Serializer* serializer) const
{
    latitude.serialize(serializer);
    longitude.serialize(serializer);
}

void Geo_coordinates::deserialize(BASE::Deserializer* deserializer)
{
    latitude.deserialize(deserializer);
    longitude.deserialize(deserializer);
}

bool Geo_coordinates::operator==(const Geo_coordinates& obj) const
{
    return latitude == obj.latitude && longitude == obj.longitude;
}

string Geo_coordinates::to_string(Coordinate_format format) const
{
    stringstream stream;
    stream << latitude.to_string(format) << "," << longitude.to_string(format);
    return stream.str();
}

size_t Geo_coordinates::hash() const
{
    return latitude.hash() + longitude.hash();
}

//
// Geo_detail
//

Geo_detail::Geo_detail(const Geo_coordinates& coordinates, double radius) : coordinates(coordinates), radius(radius)
{
}

string Geo_detail::to_string(Coordinate_format format) const
{
    stringstream stream;
    const Geo_latitude& lat_a = coordinates.get_latitude() - radius;
    const Geo_longitude& lon_a = coordinates.get_longitude() - radius;
    const Geo_latitude& lat_b = coordinates.get_latitude() + radius;
    const Geo_longitude& lon_b = coordinates.get_longitude() + radius;
    stream << lon_a.to_string(format) << "," << lat_a.to_string(format) << "," << lon_b.to_string(format) << "," << lat_b.to_string(format);
    return stream.str();
}

}}
