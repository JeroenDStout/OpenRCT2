#include "vehicleex.h"
#include "ride.h"
#include "../world/sprite.h"
#include "../ride/track.h"
#include "../audio/audio.h"
#include "../config/config.h"
#include "../peep/peepex.h"

void vehicleex_per_tile(rct_vehicle *vehicle, bool forwards)
{
    vehicleex_update_crossings(vehicle, forwards);

    if (gConfigPeepEx.enable_hamelin_entertainer) {
        peepex_event_broadcast_instr e = create_peepex_event_broadcast_instr();
        e.broadcast_type = PEEPEX_BROADCAST_EVENT_GENERIC_VEHICLE;
        e.primary_vehicle   = vehicle;
        peepex_broadcast_event(&e);
    }
}

void vehicleex_update_crossings(rct_vehicle* vehicle, bool travellingForwards)
{
    if (vehicle_get_head(vehicle) != vehicle) {
        return;
    }

    rct_vehicle *frontVehicle;
    rct_vehicle *backVehicle;

    if (travellingForwards) {
        frontVehicle = vehicle;
        backVehicle = vehicle_get_tail(vehicle);
    }
    else {
        frontVehicle = vehicle_get_tail(vehicle);
        backVehicle = vehicle;
    }

    rct_xy_element xyElement;
    track_begin_end output;
    sint32 z, direction;

    xyElement.x = frontVehicle->track_x;
    xyElement.y = frontVehicle->track_y;
    z           = frontVehicle->track_z;
    xyElement.element = map_get_track_element_at_of_type_seq(
        frontVehicle->track_x, frontVehicle->track_y, frontVehicle->track_z >> 3,
        frontVehicle->track_type >> 2, 0
        );

    bool hasUsedClaxon = false;
    
    if (xyElement.element && vehicle->status != VEHICLE_STATUS_ARRIVING) {
        sint16  autoReserveAhead = 4 + abs(vehicle->velocity) / 150000;
        sint16  crossingBonus = 0;

            // vehicle positions mean we have to take larger
            //  margins for travelling backwards
        if (!travellingForwards)
            autoReserveAhead += 1;

        while (true) {
            rct_map_element *mapElement = map_get_path_element_at(
               xyElement.x / 32,
               xyElement.y / 32,
               xyElement.element->base_height
            );

            if (mapElement) {
                crossingBonus = 4;
                if (!hasUsedClaxon && !(mapElement->flags & MAP_ELEMENT_FLAG_TEMPORARILY_BLOCKED)) {
                    vehicleex_claxon(vehicle);
                    hasUsedClaxon = true;
                }
                mapElement->flags |= MAP_ELEMENT_FLAG_TEMPORARILY_BLOCKED;
            }
            else {
                crossingBonus = 0;
            }

            if (--autoReserveAhead + crossingBonus <= 0)
                break;

            z = xyElement.element->base_height;

            if (travellingForwards) {
                if (!track_block_get_next(&xyElement, &xyElement, &z, &direction)) {
                    break;
                }
            }
            else {
                if (!track_block_get_previous(xyElement.x, xyElement.y, xyElement.element, &output)) {
                    break;
                }
                xyElement.x = output.begin_x;
                xyElement.y = output.begin_y;
                xyElement.element = output.begin_element;
            }

            if (xyElement.element->properties.track.type == TRACK_ELEM_BEGIN_STATION ||
                xyElement.element->properties.track.type == TRACK_ELEM_MIDDLE_STATION ||
                xyElement.element->properties.track.type == TRACK_ELEM_END_STATION) {
                break;
            }
        }
    }

    xyElement.x = backVehicle->track_x;
    xyElement.y = backVehicle->track_y;
    z           = backVehicle->track_z;
    xyElement.element = map_get_track_element_at_of_type_seq(
        backVehicle->track_x, backVehicle->track_y, backVehicle->track_z >> 3,
        backVehicle->track_type >> 2, 0
        );

    if (xyElement.element) {
        uint8 freeCount = travellingForwards? 3 : 1;

        while (freeCount-- > 0) {
            if (travellingForwards) {
                if (track_block_get_previous(xyElement.x, xyElement.y, xyElement.element, &output)) {
                    xyElement.x = output.begin_x;
                    xyElement.y = output.begin_y;
                    xyElement.element = output.begin_element;
                }
            }
        
            rct_map_element *mapElement = map_get_path_element_at(
                xyElement.x / 32,
                xyElement.y / 32,
                xyElement.element->base_height
            );
            if (mapElement) {
                mapElement->flags &= ~MAP_ELEMENT_FLAG_TEMPORARILY_BLOCKED;
            }
        }
    }
}

void vehicleex_claxon(rct_vehicle *vehicle)
{
    rct_ride_entry* rideEntry = get_ride_entry(vehicle->ride_subtype);
    audio_play_sound_at_location((rideEntry->vehicles[0].sound_range == 3)? SOUND_TRAIN_WHISTLE : SOUND_TRAM, vehicle->x, vehicle->y, vehicle->z);
}