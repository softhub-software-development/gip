
/*
 *  units.h
 *
 *  Created by Christian Lehner on 8/16/10.
 *  Copyright 2018 softhub. All rights reserved.
 */

#ifndef LIB_UTIL_UNITS_H
#define LIB_UTIL_UNITS_H

#include <base/base.h>
#include <string>

namespace SOFTHUB {
namespace UTIL {

typedef enum {
    inch, metric
} Unit_system;

const double MILLIMETERS_PER_INCH = 25.4;
const double INCHES_PER_MILLIMETER = 1 / MILLIMETERS_PER_INCH;
// the official conversion factor from feet to meters
const double FEET_PER_METER = 3.28083333333;
const double METERS_PER_FOOT = 1.0 / FEET_PER_METER;
// conversion factor of statute miles to meters from wikipedia
const double METERS_PER_STATUTE_MILE = 1609.344;
const double STATUTE_MILES_PER_METER = 1.0 / METERS_PER_STATUTE_MILE;
// conversion factor of nautical miles to meters from wikipedia
const double METERS_PER_NAUTICAL_MILE = 1852;
const double NAUTICAL_MILES_PER_METER = 1.0 / METERS_PER_NAUTICAL_MILE;
// conversion factor of statute miles to feet from wikipedia
const double FEET_PER_STATUTE_MILE = 5280;
const double STATUTE_MILES_PER_FOOT = 1.0 / FEET_PER_STATUTE_MILE;
// conversion factor of nautical miles to feet from wikipedia
const double FEET_PER_NAUTICAL_MILE = 6076.115;
const double NAUTICAL_MILES_PER_FOOT = 1.0 / FEET_PER_NAUTICAL_MILE;
// conversion factor from statute to nautical miles
const double NAUTICAL_MILES_PER_STATUTE_MILE = METERS_PER_NAUTICAL_MILE * STATUTE_MILES_PER_METER;
// conversion factor from nautical to statute miles
const double STATUTE_MILES_PER_NAUTICAL_MILE = METERS_PER_STATUTE_MILE * NAUTICAL_MILES_PER_METER;

FORWARD_CLASS(Units);

class Units : public BASE::Object<> {

    typedef BASE::Hash_map<std::string,Units_ref> Units_map;

    static Units_map unit_map;

    friend class Util_module;

    static void init();

public:
    static double to_millimeters(Unit_system system, double value);
    static double to_inches(Unit_system system, double value);

    static double millimeters_to_inches(double millimeters) { return INCHES_PER_MILLIMETER * millimeters; }
    static double inches_to_millimeters(double inches) { return MILLIMETERS_PER_INCH * inches; }

    static double meters_to_feet(double meters) { return FEET_PER_METER * meters; }
    static double feet_to_meters(double feet) { return METERS_PER_FOOT * feet; }

    static double meters_to_statute_miles(double meters) { return STATUTE_MILES_PER_METER * meters; }
    static double statute_miles_to_meters(double miles) { return METERS_PER_STATUTE_MILE * miles; }

    static double meters_to_nautical_miles(double meters) { return NAUTICAL_MILES_PER_METER * meters; }
    static double nautical_miles_to_meters(double miles) { return METERS_PER_NAUTICAL_MILE * miles; }

    static double feet_to_statute_miles(double feet) { return STATUTE_MILES_PER_FOOT * feet; }
    static double statute_miles_to_feet(double miles) { return FEET_PER_STATUTE_MILE * miles; }

    static double feet_to_nautical_miles(double feet) { return NAUTICAL_MILES_PER_FOOT * feet; }
    static double nautical_miles_to_feet(double miles) { return FEET_PER_NAUTICAL_MILE * miles; }

    static double statute_miles_to_nautical_miles(double miles) { return STATUTE_MILES_PER_NAUTICAL_MILE * miles; }
    static double nautical_miles_to_statute_miles(double miles) { return NAUTICAL_MILES_PER_STATUTE_MILE * miles; }

    static double knots_to_kilometers_per_hour(double knots) { return METERS_PER_NAUTICAL_MILE / 1000 * knots; }
    static double kilometers_per_hour_to_knots(double kmh) { return NAUTICAL_MILES_PER_METER * 1000 * kmh; }

    static double kilometers_per_hour_to_miles_per_hour(double kmh) { return STATUTE_MILES_PER_METER * 1000 * kmh; }
    static double miles_per_hour_to_kilometers_per_hour(double mph) { return METERS_PER_STATUTE_MILE / 1000 * mph; }

    static double knots_to_miles_per_hour(double knots) { return NAUTICAL_MILES_PER_STATUTE_MILE * knots; }
    static double miles_per_hour_to_knots(double mph) { return STATUTE_MILES_PER_NAUTICAL_MILE * mph; }

    static Units_ref get_units(const std::string& unit_name);

    static double to_meters(double value, const std::string& unit_name);
    static double to_statute_miles(double value, const std::string& unit_name);
    static double to_nautical_miles(double value, const std::string& unit_name);
    static double to_knots(double value, const std::string& unit_name);

    static double kilometers_per_hour_to_units(double value, const std::string& unit_name);
    static double miles_per_hour_to_units(double value, const std::string& unit_name);
    static double knots_to_units(double value, const std::string& unit_name);

    virtual const std::string get_name() const = 0;
    virtual const std::string get_distance_unit_name() const = 0;
    virtual const std::string get_speed_unit_name() const = 0;
    virtual double to_feet(double value) const = 0;
    virtual double to_meters(double value) const = 0;
    virtual double to_statute_miles(double value) const = 0;
    virtual double to_nautical_miles(double value) const = 0;
    virtual double to_kilometers_per_hour(double value) const = 0;
    virtual double to_knots(double value) const = 0;
    virtual double to_miles_per_hour(double value) const = 0;
    virtual double from_unit_system(Unit_system units, double value) const = 0;
    virtual double from_kilometers_per_hour(double kmh) const = 0;
    virtual double from_miles_per_hour(double mph) const = 0;
    virtual double from_knots(double knots) const = 0;
};

class Metric_units : public Units {

public:
    const std::string get_name() const { return "metric"; }
    const std::string get_distance_unit_name() const { return "m"; }
    const std::string get_speed_unit_name() const { return "kmh"; }
    double to_feet(double value) const { return meters_to_feet(value); }
    double to_meters(double value) const { return value; }
    double to_statute_miles(double value) const { return meters_to_statute_miles(value); }
    double to_nautical_miles(double value) const { return meters_to_nautical_miles(value); }
    double to_kilometers_per_hour(double value) const { return value; }
    double to_knots(double value) const { return kilometers_per_hour_to_knots(value); }
    double to_miles_per_hour(double value) const { return kilometers_per_hour_to_miles_per_hour(value); }
    double from_unit_system(Unit_system units, double value) const;
    double from_kilometers_per_hour(double kmh) const { return kmh; }
    double from_miles_per_hour(double mph) const { return miles_per_hour_to_kilometers_per_hour(mph); }
    double from_knots(double knots) const { return knots_to_kilometers_per_hour(knots); }

    DECLARE_CLASS('sgmu');
};

class Imperial_units : public Units {

public:
    const std::string get_name() const { return "imperial"; }
    const std::string get_distance_unit_name() const { return "mi"; }
    const std::string get_speed_unit_name() const { return "mph"; }
    double to_feet(double value) const { return statute_miles_to_feet(value); }
    double to_meters(double value) const { return statute_miles_to_meters(value); }
    double to_statute_miles(double value) const { return value; }
    double to_nautical_miles(double value) const { return statute_miles_to_nautical_miles(value); }
    double to_kilometers_per_hour(double value) const { return miles_per_hour_to_kilometers_per_hour(value); }
    double to_knots(double value) const { return miles_per_hour_to_knots(value); }
    double to_miles_per_hour(double value) const { return value; }
    double from_unit_system(Unit_system units, double value) const;
    double from_kilometers_per_hour(double kmh) const { return kilometers_per_hour_to_miles_per_hour(kmh); }
    double from_miles_per_hour(double mph) const { return mph; }
    double from_knots(double knots) const { return knots_to_miles_per_hour(knots); }

    DECLARE_CLASS('sgiu');
};

class Nautical_units : public Units {

public:
    const std::string get_name() const { return "nautical"; }
    const std::string get_distance_unit_name() const { return "nm"; }
    const std::string get_speed_unit_name() const { return "kts"; }
    double to_feet(double value) const { return nautical_miles_to_feet(value); }
    double to_meters(double value) const { return nautical_miles_to_meters(value); }
    double to_statute_miles(double value) const { return nautical_miles_to_statute_miles(value); }
    double to_nautical_miles(double value) const { return value; }
    double to_kilometers_per_hour(double value) const { return knots_to_kilometers_per_hour(value); }
    double to_knots(double value) const { return value; }
    double to_miles_per_hour(double value) const { return knots_to_miles_per_hour(value); }
    double from_unit_system(Unit_system units, double value) const;
    double from_kilometers_per_hour(double kmh) const { return kilometers_per_hour_to_knots(kmh); }
    double from_miles_per_hour(double mph) const { return miles_per_hour_to_knots(mph); }
    double from_knots(double knots) const { return knots; }

    DECLARE_CLASS('sgnu');
};

}}

#endif
