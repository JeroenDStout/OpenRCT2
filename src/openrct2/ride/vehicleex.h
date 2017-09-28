#pragma region Copyright (c) 2014-2017 OpenRCT2 Developers
/*****************************************************************************
 * OpenRCT2, an open source clone of Roller Coaster Tycoon 2.
 *
 * OpenRCT2 is the work of many authors, a full list can be found in contributors.md
 * For more information, visit https://github.com/OpenRCT2/OpenRCT2
 *
 * OpenRCT2 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * A full copy of the GNU General Public License can be found in licence.txt
 *****************************************************************************/
#pragma endregion

#ifndef _VEHICLEEX_H_
#define _VEHICLEEX_H_

#include "vehicle.h"

//void vehicleex_upon_depart(rct_vehicle* vehicle);
void vehicleex_per_tile(rct_vehicle* vehicle, bool forwards);
void vehicleex_per_splash(rct_vehicle* vehicle);
void vehicleex_update_crossings(rct_vehicle* vehicle, bool forwards);
void vehicleex_claxon(rct_vehicle*);

#endif