
/*
 *  units.cpp
 *
 *  Created by Christian Lehner on 8/16/10.
 *  Copyright 2018 softhub. All rights reserved.
 */

#include "stdafx.h"
#include "util_units.h"

namespace SOFTHUB {
namespace UTIL {

Units::Units_map Units::unit_map;

void Units::init()
{
    if (!unit_map.empty())
        return;
    Units_ref metric_units = new Metric_units();
    unit_map.insert(metric_units->get_name(), metric_units);
    unit_map.insert(metric_units->get_distance_unit_name(), metric_units);
    unit_map.insert(metric_units->get_speed_unit_name(), metric_units);
    Units_ref statute_units = new Imperial_units();
    unit_map.insert(statute_units->get_name(), statute_units);
    unit_map.insert(statute_units->get_distance_unit_name(), statute_units);
    unit_map.insert(statute_units->get_speed_unit_name(), statute_units);
    Units_ref nautical_units = new Nautical_units();
    unit_map.insert(nautical_units->get_name(), nautical_units);
    unit_map.insert(nautical_units->get_distance_unit_name(), nautical_units);
    unit_map.insert(nautical_units->get_speed_unit_name(), nautical_units);
    unit_map.insert("knots", nautical_units);
}

double Units::to_millimeters(Unit_system system, double value)
{
    switch (system) {
    case inch:
        return inches_to_millimeters(value);
    case metric:
        return value;
    }
    assert(0);
    return 0;
}

double Units::to_inches(Unit_system system, double value)
{
    switch (system) {
    case inch:
        return value;
    case metric:
        return millimeters_to_inches(value);
    }
    assert(0);
    return 0;
}

Units_ref Units::get_units(const std::string& unit_name)
{
    Units_map::iterator it = unit_map.find(unit_name);
    if (it == unit_map.end())
        return 0;
    return it->second;
}

double Units::to_meters(double value, const std::string& unit_name)
{
    Units_ref units = get_units(unit_name);
    return units ? units->to_meters(value) : 0;
}

double Units::to_statute_miles(double value, const std::string& unit_name)
{
    Units_ref units = get_units(unit_name);
    return units ? units->to_statute_miles(value) : 0;
}

double Units::to_nautical_miles(double value, const std::string& unit_name)
{
    Units_ref units = get_units(unit_name);
    return units ? units->to_nautical_miles(value) : 0;
}

double Units::to_knots(double value, const std::string& unit_name)
{
    Units_ref units = get_units(unit_name);
    return units ? units->to_knots(value) : 0;
}

double Units::kilometers_per_hour_to_units(double value, const std::string& unit_name)
{
    Units_ref units = get_units(unit_name);
    return units ? units->from_kilometers_per_hour(value) : 0;
}

double Units::miles_per_hour_to_units(double value, const std::string& unit_name)
{
    Units_ref units = get_units(unit_name);
    return units ? units->from_miles_per_hour(value) : 0;
}

double Units::knots_to_units(double value, const std::string& unit_name)
{
    Units_ref units = get_units(unit_name);
    return units ? units->from_knots(value) : 0;
}

double Metric_units::from_unit_system(Unit_system units, double value) const
{
    return to_millimeters(units, value);
}

double Imperial_units::from_unit_system(Unit_system units, double value) const
{
    return to_inches(units, value);
}

double Nautical_units::from_unit_system(Unit_system units, double value) const
{
    return to_inches(units, value);
}

}}
