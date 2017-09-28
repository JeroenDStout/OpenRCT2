#include "peepex.h"
#include "../ride/ride.h"
#include "../world/footpath.h"
#include "../scenario/scenario.h"
#include "../config/Config.h"
#include "../audio/audio.h"

const rct_xy16 peepex_directional_offset[4] = {
    { -1,  0 },
    {  0,  1 },
    {  1,  0 },
    {  0, -1 }
};

    /*  Base function for making peeps follow a target while in walking behaviour
     *  Ideally a peep has a clean shot at getting to the target, in which case
     *   a small gradient descend algorithm pushes them towards them with a dead zone
     *   and some basic avoidance of nearby other peeps.
     *  Occasionally the algorithm checks whether a peep can reach their target by
     *   the naieve method of heading towards them. If they cannot, they can get
     *   a hint from a cleverer algorithm and switch to a more regular walking pattern.
     *  If the cleverer algorithm cannot find the target, the function will mark
     *   the target as lost.
     */

void peepex_update_following(rct_peep *peep, peepex_follow_instr* instr)
{
    instr->out_target_lost = false;
    
	rct_map_element *mapElement;
    bool checkBySliding = false;

    rct_xyz16 walkTarget;

    if (instr->target_peep) {
        walkTarget.x = instr->target_peep->x;
        walkTarget.y = instr->target_peep->y;
        walkTarget.z = instr->target_peep->z;
    }
    else {
        walkTarget.x = instr->target_fluid.x;
        walkTarget.y = instr->target_fluid.y;
        walkTarget.z = instr->target_fluid.z;
    }
    
    sint32 offsetX      = walkTarget.x - peep->x;
    sint32 offsetY      = walkTarget.y - peep->y;

    sint32 distancePow  = offsetX * offsetX + offsetY * offsetY;

    bool   targetIsSuspect = false;

    instr->out_effective_distance   = distancePow;
    instr->out_comfortable_position = false;

        // find the direction which faces the peep

    instr->out_facing_direction     = peepex_direction_from_xy(offsetX, offsetY);

    if (peep->peepex_flags_tmp & PEEPEX_TMP_FOLLOW_FLAG_FREE_WALK) {
        // peepex_following_flags will have 0x1 set when-ever we are naively chasing our target

            // We estimate a score for the target location using a bit of gradient
            //  descend magic!

    #define INSTANCES 3

            // Create locations around the peep's current destination which will be checked.
            //  If they are better, the peep will adjust their destination.

        uint8 instanceCount = INSTANCES;
        rct_xy16 location_pos[INSTANCES];
        rct_xy16 location_push;
        sint32 location_cost[INSTANCES] = { 0, 0, 0 };
        uint32  randomSeed       = scenario_rand_max(8);
        sint32  randomFlipped    = (randomSeed & 1)? 1 : -1;
        sint32  randomOffset     = (1 + (randomSeed >> 1)) * randomFlipped;
        rct_xy16 walkTargetAimed = { walkTarget.x, walkTarget.y };

        if (instr->target_peep && instr->target_forward_offset != 0) {
            walkTargetAimed.x += peepex_directional_offset[instr->target_peep->sprite_direction / 8].x * instr->target_forward_offset;
            walkTargetAimed.y += peepex_directional_offset[instr->target_peep->sprite_direction / 8].y * instr->target_forward_offset;
        }
    
        location_pos[0].x = peep->destination_x;
        location_pos[0].y = peep->destination_y;
        location_pos[1].x = peep->destination_x + randomOffset;
        location_pos[1].y = peep->destination_y;
        location_pos[2].x = peep->destination_x;
        location_pos[2].y = peep->destination_y + randomOffset;

            // first judge the locations based on the distance

        for (uint8 i = 0; i < instanceCount; i++) {
            offsetX      = walkTargetAimed.x - location_pos[i].x;
            offsetY      = walkTargetAimed.y - location_pos[i].y;
            distancePow  = offsetX * offsetX + offsetY * offsetY;
        
            location_cost[i] += (distancePow * instr->base_gradient_weight_in_percent) / 1000;

                // Punish being too far away

            if (distancePow > instr->attempt_max_distance) {
                location_cost[i] += (distancePow - instr->attempt_max_distance) * 100;
            }

            offsetX      = walkTarget.x - location_pos[i].x;
            offsetY      = walkTarget.y - location_pos[i].y;
            distancePow  = offsetX * offsetX + offsetY * offsetY;
        
                // Punish being too close to the actual target

            if (distancePow < instr->attempt_min_distance) {
                location_cost[i] += (instr->attempt_min_distance - distancePow) * 200;
            }

            if (instr->target_peep) {
                offsetX      = instr->target_peep->destination_x - location_pos[i].x;
                offsetY      = instr->target_peep->destination_y - location_pos[i].y;
                distancePow  = offsetX * offsetX + offsetY * offsetY;
        
                    // Punish being too close to the target's expected location

                if (distancePow < instr->attempt_min_distance) {
                    location_cost[i] += (instr->attempt_min_distance - distancePow) * 75;
                }
            }

                // Punish having to walk

            offsetX      = peep->x - location_pos[i].x;
            offsetY      = peep->y - location_pos[i].y;
            distancePow  = offsetX * offsetX + offsetY * offsetY;

            if (distancePow > 256 * 256 && i == 0) {
                targetIsSuspect = true;
            }

            location_cost[i] += distancePow / 2;
        }

            // Check if our target location is very crowded
        
        uint16 firstSprites[64];
        uint32 firstSpritesCount;
        sint16 peepFindRange = 32;
        rct_sprite *sprite;
        firstSpritesCount = sprite_get_first_in_multiple_quadrants(peep->x - peepFindRange, peep->y - peepFindRange, peep->x + peepFindRange, peep->y + peepFindRange, firstSprites, 64);
		for (uint32 index = 0; index < firstSpritesCount; index++) {
            uint16 sprite_id = firstSprites[index];
	        for (; sprite_id != SPRITE_INDEX_NULL; sprite_id = sprite->unknown.next_in_quadrant){
		        sprite = get_sprite(sprite_id);

                if (sprite->unknown.sprite_identifier != SPRITE_IDENTIFIER_PEEP)
                    continue;
        
	            rct_peep* other_peep = (rct_peep*)sprite;

                if (other_peep == peep)
                    continue;

                    // Mildly punish for peeps nearby, harsly punish for peeps very close

                sint32 effectiveWeight = instr->crowd_weight_in_percent;
                if (other_peep->state == PEEP_STATE_WALKING ||
                    other_peep->state == PEEP_STATE_PATROLLING) {
                    effectiveWeight *= instr->moving_crowd_weight_in_percent;
                }
                else {
                    effectiveWeight *= 100;
                }

                for (uint8 i = 0; i < instanceCount; i++) {
		            sint16 distX = other_peep->x - location_pos[i].x;
		            sint16 distY = other_peep->y - location_pos[i].y;
		            sint32 workDist = distX * distX + distY * distY;

                    if (workDist < 256) {
                        location_cost[i]        += ((256 - workDist) * effectiveWeight) / 320000;
                        if (workDist < 36) {
                            location_cost[i]    += ((36 - workDist) * effectiveWeight) / 400;
                        }
                    }
                }
            }
        }

            // Find the gradient

        location_push.x = 0;
        location_push.y = 0;

        location_cost[0] -= 16;

        sint16 offset = 4;

        if (location_cost[0] > location_cost[1]) {
            location_push.x = offset * randomFlipped;
        }
        if (location_cost[0] > location_cost[2]) {
            location_push.y = offset * randomFlipped;
        }

            // Push the destination, if applicable

        if (!targetIsSuspect) {
            peep->destination_x += location_push.x;
            peep->destination_y += location_push.y;
            peep->destination_tolerence = 4;
        }
        else {
            peep->destination_x = walkTarget.x;
            peep->destination_y = walkTarget.y;
            peep->destination_tolerence = 4;
        }

            // Make sure our position is allowed based on the tile rules
            //  We look at the current tile to see if the edges allow us to move there
        
	    mapElement = map_get_path_element_below_or_at(peep->x / 32, peep->y / 32, peep->z >> 3);
	    if (mapElement) {
            rct_xy16 dest = peepex_pathing_clamp_path_regular(mapElement, peep, peep->x, peep->y, peep->z, peep->destination_x, peep->destination_y, 0, 5, 12);
            peep->destination_x = dest.x;
            peep->destination_y = dest.y;
        }
        else {
            peep->destination_x = peep->x;
            peep->destination_y = peep->y;
        }
    }

        // Implement movement. This can both be the gradiend descend method or having the peep
        //  walking towards an otherwise found target.

    sint16 x, y, z, xy_distance;
    
    if (peep_update_action(&x, &y, &xy_distance, peep)){
            // We have some distance to move, so handle paths and slopes
	    mapElement = map_get_path_element_below_or_at(x / 32, y / 32, (peep->z >> 3) + 2);
        if (mapElement) {
            peep->next_z        = mapElement->base_height;
            peep->next_var_29   = mapElement->properties.path.type & 7;
        }
        z = peep_get_height_on_slope(peep, x, y);
        sprite_move(x, y, z, (rct_sprite*)peep);
    }
    else {
            // We have reached our destination. If we were not walking freely, redo
            //  the slide check to see if we can switch to free behaviour or whether we
            //  should get another hint
        if (!(peep->peepex_flags_tmp & PEEPEX_TMP_FOLLOW_FLAG_FREE_WALK))
            checkBySliding = true;
        instr->out_comfortable_position = peep->peepex_flags_tmp & PEEPEX_TMP_FOLLOW_FLAG_FREE_WALK;
    }

        // Every so-many ticks force the slide check so peeps cannot get easily get stuck
    if ((gScenarioTicks + peep->id) % 31 == 0) {
        checkBySliding = true;
    }

    checkBySliding = true;

    if (checkBySliding) {
            // Do the slide check

        peepex_sliding_check_instr  slidingInstr;

        slidingInstr.peep               = 0;
        slidingInstr.current.x          = peep->x;
        slidingInstr.current.y          = peep->y;
        slidingInstr.current.z          = peep->z;
        slidingInstr.target.x           = walkTarget.x;
        slidingInstr.target.y           = walkTarget.y;
        slidingInstr.target.z           = walkTarget.z;
        slidingInstr.max_xy_distance    = instr->min_tile_nearness;    // we need to reach the actual tile
        slidingInstr.max_z_distance     = 2;    // allow for slopes

        peepex_sliding_check(&slidingInstr);

        if (!slidingInstr.out_target_is_reachable) {
                // We could not find the location by sliding, so potentially get a smarter hint

            peepex_pathing_hint_instr hintInstr;
            
            hintInstr.current.x         = peep->x;
            hintInstr.current.y         = peep->y;
            hintInstr.current.z         = peep->z;
            hintInstr.target.x          = walkTarget.x;
            hintInstr.target.y          = walkTarget.y;
            hintInstr.target.z          = walkTarget.z;
            hintInstr.max_search_depth  = 5;

            peepex_pathing_hint(&hintInstr);

            if (hintInstr.out_found_target) {
                assert(hintInstr.out_primary_direction <= 3);
                x = peep->next_x;
                y = peep->next_y;
                x += TileDirectionDelta[hintInstr.out_primary_direction].x;
                y += TileDirectionDelta[hintInstr.out_primary_direction].y;
                peep_move_one_tile_messy(x, y, hintInstr.out_primary_direction, peep);
                peep->peepex_flags_tmp &= ~0x1;
            }
            else {
                instr->out_target_lost  = true;
            }
        }
        else {
            if (slidingInstr.out_potential_block) {
                log_warning("potential");
                peep->peepex_flags_tmp &= ~0x1;
                x = peep->x & 0xFFE0;
                y = peep->y & 0xFFE0;
                x += TileDirectionDelta[slidingInstr.out_first_step_direction].x;
                y += TileDirectionDelta[slidingInstr.out_first_step_direction].y;
                peep_move_one_tile_messy(x, y, slidingInstr.out_first_step_direction, peep);
            }
            else {
                    // We can reach the destination, use direct pathing
                peep->peepex_flags_tmp |= 0x1;
            }
        }
    }

        // If we are reasonably close have a large desire to face the target

    if (xy_distance < 8 + (sint16)((peep->id + gScenarioTicks) % 7))
        peep->sprite_direction = instr->out_facing_direction * 8;
}

    /*  The slide check makes sure a peep can get from point A to B without
     *   complex path-finding. This basically means that if they take the
     *   simplest route (always step towards the target) they will probably reach it.
     *  Doing this check makes it possible to check if peeps can reach their target
     *   across a square or wide path or simple to navigate set of walls.
     *  FI, the follow function uses this to regularly check if a peep should still be
     *   trying to reach an entertainer, or whether doing so would get them stuck.
     *  By setting the xy and z distance above 0, peeps can follow targets which are off-path,
     *   such as potentially following vehicles. Z distance should be at least 2 to allow for
     *   following on slopes.
     */

void peepex_sliding_check(peepex_sliding_check_instr* instr)
{
    rct_xyz16 current;
    uint8 lastDirection = 0xFF;
    
    current.x = instr->current.x;
    current.y = instr->current.y;
    current.z = instr->current.z;

    instr->out_potential_block  = false;

    uint8 loops = 0;

    while (true) {
            // Are we close enough?
        if (abs((current.x >> 5) - (instr->target.x >> 5)) <= instr->max_xy_distance &&
            abs((current.y >> 5) - (instr->target.y >> 5)) <= instr->max_xy_distance) {
            if (abs((current.z >> 3) - (instr->target.z >> 3)) > instr->max_z_distance) {
                    // we're at the right xy but wrong z - we can never reach by sliding
                break;
            }
    
            instr->out_target_is_reachable = true;
            return;
        }

	    rct_map_element *mapElement = map_get_path_element_below_or_at(current.x >> 5, current.y >> 5, current.z >> 3);
        if (!mapElement) {
                // We are not on a path, so always fail
            break;
        }

            // Find our offset
        sint16 offsetX = instr->target.x - current.x;
        sint16 offsetY = instr->target.y - current.y;
        uint8 direction, immediateDirection;

        immediateDirection = direction = peepex_direction_from_xy(offsetX, offsetY);
            // If we cannot find a path right away, try the less urgent direction as well
        if (!peepex_find_connected_path(instr->peep, mapElement, current.x, current.y, direction, &current.z)) {
            if (direction & 0x1) {
                if (offsetY == 0)
                    break;
                direction = (offsetY > 0)? 1 : 3;
            }
            else {
                if (offsetX == 0)
                    break;
                direction = (offsetX < 0)? 0 : 2;
            }
            if (!peepex_find_connected_path(instr->peep, mapElement, current.x, current.y, direction, &current.z)) {
                break;
            }
        }

        if (loops == 0) {
            instr->out_potential_block      = immediateDirection != direction;
            if (instr->out_potential_block) log_warning("hm %i %i", immediateDirection, direction);
            instr->out_first_step_direction = direction;
        }

            // Make sure we don't get stuck. Peeps walking from very far away would need a great number of checks. Currently this is
            //  set to the (high) value of 32 for testing purposes.
        if (++loops > 16) {
            log_warning("Very long sliding check (%i) for peep at [%i, %i, %i] aiming for [%i, %i, %i], dir %i, prev %i",
                loops, current.x, current.y, current.z, instr->target.x, instr->target.y, instr->target.z, direction, lastDirection);
            if (loops > 32)
                break;
        }

            // if we're heading back relative to the last slide step,
            //  that means we are getting into a loop and the person is unreacheable
        if (lastDirection == ((direction + 2) % 4))
            break;
        lastDirection = direction;

            // clamp our next position to the edges of the next tile our direction would take us
        switch (direction) {
        case 0:
            current.x = (current.x & 0xFFE0) - 6;
            break;
        case 1:
            current.y = (current.y & 0xFFE0) + 38;
            break;
        case 2:
            current.x = (current.x & 0xFFE0) + 38;
            break;
        case 3:
            current.y = (current.y & 0xFFE0) - 6;
            break;
        };
    }
    
        // we failed
    instr->out_target_is_reachable  = false;
}

void peepex_pathing_hint(peepex_pathing_hint_instr* instr)
{
        // there is no smart hint (TODO), so always fail

    instr->out_found_target         = false;
}

bool peepex_find_connected_path(rct_peep *peep, rct_map_element *element, sint16 x, sint16 y, uint8 direction, sint16 *nextZ)
{
    log_warning("try %i %i", x >> 5, y >> 5);
    if (!(element->properties.path.edges & (1 << direction))) {
        log_warning("fail");
        return false;
    }

    if (!peep || peep->type != PEEP_TYPE_STAFF) {
        rct_map_element *bannerElement = get_banner_on_path(element);
        if (bannerElement != NULL) {
            do {
                if (0 == (bannerElement->properties.banner.flags & (1 << direction)))
                    return false;
            } while ((bannerElement = get_banner_on_path(bannerElement)) != NULL);
        }
    }

    // we already know the next element exists (the edge exists) so we just find the tile which we could link to if were are flat or sloped
    
	rct_map_element *mapElement = map_get_path_element_below_or_at((x + TileDirectionDelta[direction].x) >> 5,
                                                                   (y + TileDirectionDelta[direction].y) >> 5, element->base_height + 2);

    if (!mapElement)
        return false;

    *nextZ = mapElement->base_height << 3;
    return true;
}

uint8 peepex_direction_from_xy(sint16 x, sint16 y)
{
    if (abs(x) > abs(y)) {
        if (x < 0)
            return 0;
        return 2;
    }
    if (y > 0)
        return 1;
    return 3;
}

peepex_follow_instr create_peepex_follow_instr()
{
    peepex_follow_instr instr;
    
    instr.target_peep                       = 0;

    instr.attempt_min_distance              = 32*32;
    instr.attempt_max_distance              = 32*32;
    instr.target_forward_offset             = 0;
    instr.out_facing_direction              = 0;
    instr.crowd_weight_in_percent           = 100;
    instr.moving_crowd_weight_in_percent    = 100;
    instr.base_gradient_weight_in_percent   = 100;
    instr.min_tile_nearness                 = 0;

    return instr;
}