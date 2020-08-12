
/*
 *  geo_coordinate.h
 *
 *  Created by Christian Lehner on 8/9/10.
 *  Copyright (c) 2019 Softhub. All rights reserved.
 *
 */

#ifndef GEOGRAPHY_GEO_COORDINATE_H
#define GEOGRAPHY_GEO_COORDINATE_H

#include <base/base.h>
#include <string>

namespace SOFTHUB {
namespace GEOGRAPHY {

typedef enum {
    unspecified = 0, north, east, south, west
} Hemisphere;

typedef enum {
    standard = 0, with_units, decimal
} Coordinate_format;

//
// Geo_coordinate
//

class Geo_coordinate {

protected:
    short hours;
    short minutes;
    float seconds;
    Hemisphere hemisphere;

    Geo_coordinate() : hours(0), minutes(0), seconds(), hemisphere(unspecified) {}

public:
    Geo_coordinate(int h, int m, double s, Hemisphere hemi) : hours(short(h)), minutes(short(m)), seconds(float(s)), hemisphere(hemi) {}

    template <typename T> T to_degrees() const;
    template <typename T> T to_radians() const;
    template <typename T> static T to_radians(double degree);

    short get_hours() const { return hours; }
    short get_minutes() const { return minutes; }
    float get_seconds() const { return seconds; }
    Hemisphere get_hemisphere() const { return hemisphere; }
    void serialize(BASE::Serializer* serializer) const;
    void deserialize(BASE::Deserializer* deserializer);
    bool operator==(const Geo_coordinate& obj) const;
    size_t hash() const;
    std::string coordinate_format(Coordinate_format format) const;
    virtual std::string to_string(Coordinate_format format = standard) const = 0;

    static char short_hemisphere(Hemisphere hemisphere);
    static int sign_of_hemisphere(Hemisphere hemisphere);
    static Hemisphere hemisphere_from(char c);
};

//
// Geo_latitude
//

class Geo_latitude : public Geo_coordinate {

public:
    Geo_latitude() {}
    Geo_latitude(int h, int m, double s, Hemisphere hemi) : Geo_coordinate(h, m, s, hemi) {}

    Geo_latitude operator+(double r) const;
    Geo_latitude operator-(double r) const;
    std::string to_string(Coordinate_format format = standard) const;

    static Geo_latitude from_radians(double phi);
#ifndef NO_GEO_PARSER
    static Geo_latitude parse(const std::string& s);
#endif
};

//
// Geo_longitude
//

class Geo_longitude : public Geo_coordinate {

public:
    Geo_longitude() {}
    Geo_longitude(int h, int m, double s, Hemisphere hemi) : Geo_coordinate(h, m, s, hemi) {}

    Geo_longitude operator+(double r) const;
    Geo_longitude operator-(double r) const;
    std::string to_string(Coordinate_format format = standard) const;

    static Geo_longitude from_radians(double lambda);
#ifndef NO_GEO_PARSER
    static Geo_longitude parse(const std::string& s);
#endif
};

//
// Geo_coordinates
//

class Geo_coordinates {

    Geo_latitude latitude;
    Geo_longitude longitude;

public:
    Geo_coordinates() {}
    Geo_coordinates(const Geo_latitude& lat, const Geo_longitude lon) : latitude(lat), longitude(lon) {}

    const Geo_latitude& get_latitude() const { return latitude; }
    const Geo_longitude& get_longitude() const { return longitude; }
    void serialize(BASE::Serializer* serializer) const;
    void deserialize(BASE::Deserializer* deserializer);
    bool operator==(const Geo_coordinates& obj) const;
    bool operator!=(const Geo_coordinates& obj) const { return operator==(obj); }
    size_t hash() const;
    std::string to_string(Coordinate_format format = standard) const;

#ifndef NO_GEO_PARSER
    static Geo_coordinates parse(const std::string& s);
#endif
};

//
// Geo_detail
//

class Geo_detail {

    Geo_coordinates coordinates;
    double radius;

public:
    Geo_detail(const Geo_coordinates& coordinates, double radius);

    std::string to_string(Coordinate_format format = standard) const;
};

}}

#include "geo_coordinate_inline.h"

#endif
