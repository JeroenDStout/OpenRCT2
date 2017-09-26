#include "vehicleex.h"
#include "ride.h"
#include "../world/sprite.h"

void vehicleex_update_crossings(rct_vehicle* vehicle)
{
    if (vehicle_get_head(vehicle) != vehicle) {
        log_error("Not first");
        return;
    }

    uint16 trackType = vehicle->track_type >> 2;

    rct_xy_element xyElement;
    sint32 z, direction;

    xyElement.x = vehicle->track_x;
    xyElement.y = vehicle->track_y;
    z           = vehicle->track_z;
    xyElement.element = map_get_track_element_at_of_type_seq(
        vehicle->track_x, vehicle->track_y, vehicle->track_z >> 3,
        trackType, 0
        );

    sint16  autoReserveAhead = 2 + vehicle->velocity / 100000;
    bool    keepReserving;
    
    if (vehicle->status != VEHICLE_STATUS_ARRIVING) {
        while (true) {
            keepReserving = false;

            rct_map_element *mapElement = map_get_path_element_at(
               xyElement.x / 32,
               xyElement.y / 32,
               xyElement.element->base_height
            );

            if (mapElement) {
                mapElement->flags |= MAP_ELEMENT_FLAG_TEMPORARILY_BLOCKED;
                keepReserving = true;
            }

            if (--autoReserveAhead <= 0 && !keepReserving)
                break;

            z = xyElement.element->base_height;

            if (!track_block_get_next(&xyElement, &xyElement, &z, &direction)) {
                break;
            }
        }
    }
    
    rct_vehicle *tail = vehicle_get_tail(vehicle);

    xyElement.x = tail->track_x;
    xyElement.y = tail->track_y;
    z           = tail->track_z;
    xyElement.element = map_get_track_element_at_of_type_seq(
        tail->track_x, tail->track_y, tail->track_z >> 3,
        tail->track_type >> 2, 0
        );

    track_begin_end output;

    if (xyElement.element) {
        if (track_block_get_previous(tail->track_x, tail->track_y, xyElement.element, &output)) {
            rct_map_element *mapElement = 0;
            rct_map_element *tryMapElement = map_get_first_element_at(output.begin_x / 32, output.begin_y / 32);

            if (tryMapElement != NULL) {
                do {
                    if (map_element_get_type(tryMapElement) != MAP_ELEMENT_TYPE_PATH)
                        continue;
                    if (tryMapElement->base_height != z >> 3)
                        continue;
                    mapElement = tryMapElement;
                    break;
                } while (!map_element_is_last_for_tile(tryMapElement++));

                if (mapElement) {
                    mapElement->flags &= ~MAP_ELEMENT_FLAG_TEMPORARILY_BLOCKED;
                }
            }
        }
    }
}