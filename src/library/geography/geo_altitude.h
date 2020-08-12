
/*
 *  geo_altitude.h
 *  NavAid
 *
 *  Created by Christian Lehner on 8/17/10.
 *  Copyright (c) 2019 Softhub. All rights reserved.
 *
 */

#ifndef GEOGRAPHY_GEO_ALTITUDE_H
#define GEOGRAPHY_GEO_ALTITUDE_H

#include "geo_parse_types.h"
#include <string>

namespace SOFTHUB {
namespace GEOGRAPHY {

//typedef enum {
//    gnd = 0, msl, fl
//} Geo_altitude_reference;

class Geo_altitude {

    Altitude_reference reference;
    float value;

public:
    Geo_altitude(Altitude_reference ref, float val);

    Altitude_reference get_reference() const { return reference; }
    float get_value() const { return value; }
    float get_value_in_meters() const;
    float get_value_in_feet() const;

    static Geo_altitude parse(const std::string& s);
};

}}

#endif

/*        
        public static Altitude Parse(string str) {
            string s = str.Trim();
            RefCode reference;
            int ival = 0;
            string sval;
            if (s.Equals("UNLTD", STRCMP)) {
                reference = RefCode.GND;
                ival = int.MaxValue;
            } else if (s.Equals("BY", STRCMP) || s.Equals("L", STRCMP)) {
                reference = RefCode.GND;
            } else if (s.Equals("NOTAM", STRCMP)) {
                reference = RefCode.GND;
            } else {
                if (s.EndsWith("GND", STRCMP) || s.EndsWith("SFC", STRCMP) || s.EndsWith("AGL", STRCMP)) {
                    reference = RefCode.GND;
                    sval = s.Substring(0, s.Length - 3);
                } else if (s.EndsWith("MSL", STRCMP)) {
                    reference = RefCode.MSL;
                    sval = s.Substring(0, s.Length - 3);
                } else if (s.StartsWith("FL", STRCMP)) {
                    reference = RefCode.FL;
                    sval = s.Substring(2, s.Length - 2);
                } else {
                    reference = RefCode.GND;
                    sval = s;
                }
                if (sval.Length > 0) {
                    int slash = sval.IndexOf("/");
                    if (slash >= 0) {
                        sval = sval.Substring(0, slash);
                    }
                    if (sval.EndsWith("ft", STRCMP))
                        sval = sval.Substring(0, sval.Length - 2);
                    if (sval.EndsWith("m", STRCMP)) {
                        sval = sval.Substring(0, sval.Length - 1);
                        ival = (int) Math.Round(int.Parse(sval) * Units.FEET_PER_METER);
                    } else {
                        ival = int.Parse(sval);
                    }
                }
            }
            return new Altitude(reference, ival);
        }
*/
