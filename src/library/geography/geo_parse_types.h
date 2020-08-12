
/*
 *  geo_parse_types.h
 *
 *  Created by Christian Lehner on 8/10/10.
 *  Copyright (c) 2019 Softhub. All rights reserved.
 *
 */

#ifndef GEOGRAPHY_GEO_PARSE_TYPES_H
#define GEOGRAPHY_GEO_PARSE_TYPES_H

#include <base/base.h>

typedef enum {
	gnd = 0, msl, fl
} Altitude_reference;

typedef struct {
	Altitude_reference reference;
	float value;
} Altitude_struct;

extern Altitude_struct altitude_parse_result;
extern int parse_geo_altitude(const char* text);

typedef struct {
	int hours;
	unsigned minutes;
	float seconds;
	char hemisphere;
} Coordinate_struct;

extern Coordinate_struct coord_parse_result[2];
extern int parse_geo_latitude(const char* text);
extern int parse_geo_longitude(const char* text);
extern int parse_geo_coordinate(const char* text);

#ifdef WIN32
#define fileno _fileno
extern int isatty(int);
#endif

#endif
