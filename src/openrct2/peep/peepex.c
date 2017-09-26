#include "peepex.h"
#include "staff.h"
#include "../ride/ride.h"
#include "../world/footpath.h"
#include "../scenario/scenario.h"
#include "../config/Config.h"

/** rct2: 0x00981D7C, 0x00981D7E */
const rct_xy16 peepex_directional[4] = {
    { -2,  0 },
    {  0,  2 },
    {  2,  0 },
    {  0, -2 }
};

void peepex_base_update(rct_peep *peep, sint32 index)
{
    //if (gScenarioTicks % 11 == 0) {
        //if (peep->peepex_event_countdown > 0)
        //    peep->peepex_event_countdown -= 1;
    //}

		// If guests are idly walking and they have spent a lot of time here, make them always leave.

	//if (gConfigPeepEx.guest_max_time_in_park > 0 && peep->state == PEEP_STATE_WALKING) {

	//	uint32 time_duration = gScenarioTicks - peep->time_in_park;

	//	if ((time_duration >> 11) > (uint32)gConfigPeepEx.guest_max_time_in_park) {
	//		peep_leave_park(peep);
	//	}
	//}
}

bool peepex_update_walking_find_activity(rct_peep *peep)
{
 //   if (peep->peepex_hamelin_countdown > 0) {
 //       peep->peepex_hamelin_countdown -= 1;
 //   }
 //   
 //   const static sint32 peepFindRange           = 64;
 //   
 //   uint16 firstSprites[64];
 //   uint32 firstSpritesCount;
 //   firstSpritesCount = sprite_get_first_in_multiple_quadrants(peep->x - peepFindRange, peep->y - peepFindRange, peep->x + peepFindRange, peep->y + peepFindRange, firstSprites, 64);
	//for (uint32 i = 0; i < firstSpritesCount; i++) {
 //       uint16 sprite_id = firstSprites[i];
	//    for (rct_sprite* sprite; sprite_id != SPRITE_INDEX_NULL; sprite_id = sprite->unknown.next_in_quadrant){
	//	    sprite = get_sprite(sprite_id);

 //           if (sprite->unknown.sprite_identifier != SPRITE_IDENTIFIER_PEEP)
 //               continue;
 //       
 //           if (gConfigPeepEx.enable_hamelin_entertainer && peep->peepex_hamelin_countdown == 0 && peepex_update_walking_find_activity_hamelin(peep, sprite_id))
 //               return true;
 //       }
 //   }

    return false;
}

bool peepex_update_walking_find_activity_hamelin(rct_peep *peep, uint16 sprite_id)
{
	rct_peep* otherPeep = (rct_peep*)get_sprite(sprite_id);

    if (otherPeep->type == PEEP_TYPE_STAFF && otherPeep->staff_type == STAFF_TYPE_ENTERTAINER) {
        if (abs(otherPeep->x - peep->x) > 90)
            return false;
        if (abs(otherPeep->y - peep->y) > 90)
            return false;
        if (abs(otherPeep->z - peep->z) > 8)
            return false;

        peep->state = PEEP_STATE_EX_FOLLOWING_HAMELIN;
        peep->peepex_follow_target = sprite_id;
        peep->peepex_hamelin_countdown = 10 + scenario_rand_max(128);

        return true;
    }

    return true;
}

bool peepex_update_patrolling_find_activity(rct_peep *peep)
{
    if (!gConfigPeepEx.enable_crime)
        return false;

        // this is a try out; we know only security gets here, so let's just go with it

	uint16 sprite_id = sprite_get_first_in_quadrant(peep->x, peep->y);
	for (rct_sprite* sprite; sprite_id != SPRITE_INDEX_NULL; sprite_id = sprite->unknown.next_in_quadrant){
		sprite = get_sprite(sprite_id);
        
        if (sprite->unknown.sprite_identifier != SPRITE_IDENTIFIER_PEEP)
            continue;

		rct_peep* other_peep = (rct_peep*)sprite;

        if (other_peep->type == PEEP_TYPE_STAFF)
            continue;

            // THIS PEEP HAS BEEN FREE LONG ENOUGH, BOOK 'EM!
        peep->state = PEEP_STATE_EX_SECURITY_CHASING;
        peep->peepex_follow_target = sprite_id;
        peep->destination_x = peep->x;
        peep->destination_y = peep->y;
        peep->peepex_arrest_countdown = 0;

        log_warning("Chase 'em!");

        return true;
    }

    return false;
}

bool peepex_use_advanced_walking(rct_peep *peep)
{
    switch (peep->state) {
    case PEEP_STATE_WALKING:
    case PEEP_STATE_PATROLLING:
    case PEEP_STATE_EX_FOLLOWING_HAMELIN:
    case PEEP_STATE_EX_SECURITY_CHASING:
    case PEEP_STATE_EX_SECURITY_ESCORTING_OUT:
    case PEEP_STATE_EX_WITNESSING_EVENT:
        return true;
    }
    return false;
}

void peep_update_action_sidestepping(sint16* x, sint16* y, sint16 x_delta, sint16 y_delta, sint16* xy_distance, rct_peep* peep)
{
	int spriteDirection = 0;
	int preferenceDirection = peep->peepex_direction_preference;
	int sidestepX = 0;
	int sidestepY = 0;
			
	int x_delta_weight = x_delta + ((preferenceDirection == 16 || preferenceDirection == 0)? max(0, x_delta - peep->destination_tolerence) * 3 : 0);
	int y_delta_weight = y_delta + ((preferenceDirection == 8 || preferenceDirection == 24)? max(0, y_delta - peep->destination_tolerence) * 3 : 0);

	if (y_delta_weight > x_delta_weight) {
		spriteDirection = 8;
		if (*y >= 0) {
			spriteDirection = 24;
		}

		if (scenario_rand_max(*xy_distance) < (uint32)x_delta * 8) {
			if (*x > 1)
				sidestepX = -1;
			else if (*x < -1)
				sidestepX = 1;
		}
	}
	else {
		spriteDirection = 16;
		if (*x >= 0) {
			spriteDirection = 0;
		}
		if (scenario_rand_max(*xy_distance) < (uint32)y_delta * 8) {
			if (*y > 1)
				sidestepY = -1;
			else if (*y < -1)
				sidestepY = 1;
		}
	}

	peep->sprite_direction = preferenceDirection;
	peep->peepex_direction_preference = spriteDirection;

	sint16 actualStepX = peepex_directional[spriteDirection / 8].x;
	sint16 actualStepY = peepex_directional[spriteDirection / 8].y;

	if (sidestepX != 0) {
		actualStepY += (actualStepY & 0x8000)? 1 : -1;
	}
	else if (sidestepY != 0) {
		actualStepX += (actualStepX & 0x8000)? 1 : -1;
	}

	sint16 pushAsideX = 0;
	sint16 pushAsideY = 0;
	sint16 bumpAsideX = 0;
	sint16 bumpAsideY = 0;
	sint16 expectAsideX = 0;
	sint16 expectAsideY = 0;
	sint16 pushStepX = 0;
	sint16 pushStepY = 0;

    sint16 peepExpectedNextX    = peep->x + actualStepX + sidestepX;
    sint16 peepExpectedNextY    = peep->y + actualStepY + sidestepY;
	
	if (peepex_use_advanced_walking(peep)) {
        const static sint32 peepFindRange           = 40;

		const static sint32 checkDistance		    = 5;
		const static sint32 checkDistancePow	    = 5*5;
        
		const static sint32 checkDistanceLitter		= 4;
		const static sint32 checkDistanceLitterPow	= 4*4;

		const static sint32 checkDistanceFar	    = 32;
		const static sint32 checkDistanceFarPow	    = 32*32;

        sint16 litterCount = 0;
		sint16 crowdedCount = 0;
        sint16 followPeepWeigth = (peep->peepex_crowded_store > 5) ? -3 : 0;

        uint16 firstSprites[64];
        uint32 firstSpritesCount;
        firstSpritesCount = sprite_get_first_in_multiple_quadrants(peep->x - peepFindRange, peep->y - peepFindRange, peep->x + peepFindRange, peep->y + peepFindRange, firstSprites, 64);
		for (uint32 i = 0; i < firstSpritesCount; i++) {
            uint16 sprite_id = firstSprites[i];
	        for (rct_sprite* sprite; sprite_id != SPRITE_INDEX_NULL; sprite_id = sprite->unknown.next_in_quadrant){
			    sprite = get_sprite(sprite_id);

                if (sprite->unknown.sprite_identifier != SPRITE_IDENTIFIER_PEEP) {
                    if (sprite->unknown.sprite_identifier != SPRITE_IDENTIFIER_LITTER)
                        continue;
                    if (litterCount >= 2)
                        continue;

		            rct_litter* litter = (rct_litter*)sprite;

			        sint16 distX = litter->x - peepExpectedNextX - 2;
			        sint16 distY = litter->y - peepExpectedNextY - 2;
			        sint16 workDist = distX * distX + distY * distY;
                    
		    	    if (workDist < checkDistanceFarPow) {
				        if ((actualStepX > 0) == (distX > 0) ||
					        (actualStepY > 0) == (distY > 0)) {
					        expectAsideX	    += (checkDistanceFar - abs(distX)) * ((distX < 0)? 4 : -4);
					        expectAsideY	    += (checkDistanceFar - abs(distY)) * ((distY < 0)? 4 : -4);
				        }
				        if (workDist < checkDistanceLitterPow) {
                            litterCount         += 1;
					        pushAsideX		    += (checkDistanceLitter - abs(distX)) * ((distX < 0)? 3 : -3);
					        pushAsideY		    += (checkDistanceLitter - abs(distY)) * ((distY < 0)? 3 : -3);
				        }
                    }
                }

			    rct_peep* other_peep = (rct_peep*)sprite;

			    if (other_peep == peep)
				    continue;

			    if (abs(other_peep->z - peep->z) > 32)
				    continue;
				
			    sint16 distX = other_peep->x - peepExpectedNextX;
			    sint16 distY = other_peep->y - peepExpectedNextY;
			    sint16 workDist = distX * distX + distY * distY;
			
			    if (workDist < checkDistanceFarPow) {
				    if ((actualStepX > 0) == (distX > 0) ||
					    (actualStepY > 0) == (distY > 0)) {
					    if (other_peep->direction != peep->direction)
						    crowdedCount	+= 3;
					    expectAsideX	+= (checkDistanceFar - abs(distX)) * ((distX < 0)? 2 : -2) * ((other_peep->direction == peep->direction)? followPeepWeigth : 1);
					    expectAsideY	+= (checkDistanceFar - abs(distY)) * ((distY < 0)? 2 : -2) * ((other_peep->direction == peep->direction)? followPeepWeigth : 1);
				    }
				    if (workDist < checkDistancePow) {
					    if ((actualStepX > 0) == (distX > 0) ||
						    (actualStepY > 0) == (distY > 0)) {
						    if (other_peep->direction != peep->direction) {
							    crowdedCount	+= 8;
					            bumpAsideX		+= (checkDistance - abs(distX)) * ((distX < 0)? 1 : -1);
					            bumpAsideY		+= (checkDistance - abs(distY)) * ((distY < 0)? 1 : -1);
                            }
					    }
					    pushAsideX		+= (checkDistance - abs(distX)) * ((distX < 0)? 1 : -1);
					    pushAsideY		+= (checkDistance - abs(distY)) * ((distY < 0)? 1 : -1);
				    }
			    }
		    }
        }

        expectAsideX    += pushAsideX;
        expectAsideY    += pushAsideY;

		peep->peepex_crowded_store /= 2;
		peep->peepex_crowded_store += min(100, crowdedCount / 1);
		
		sint32 peepEdgeLimits = 0xF;

		rct_map_element *mapElement = map_get_path_element_at(peep->next_x / 32, peep->next_y / 32, peep->next_z);
		if (mapElement && map_element_get_type(mapElement) == MAP_ELEMENT_TYPE_PATH) {
			sint32 tileLimits = 12;

			sint32 edges = mapElement->properties.path.edges;// & peep->peepex_path_limits;
			if ((peep->x & 0x1F) < tileLimits && !(edges & 0x1))
				peepEdgeLimits &= ~0x1;
			if ((peep->y & 0x1F) > (32-tileLimits) && !(edges & 0x2))
				peepEdgeLimits &= ~0x2;
			if ((peep->x & 0x1F) > (32-tileLimits) && !(edges & 0x4))
				peepEdgeLimits &= ~0x4;
			if ((peep->y & 0x1F) < tileLimits && !(edges & 0x8))
				peepEdgeLimits &= ~0x8;
		}

		if (pushStepX < 0 && !(peepEdgeLimits & 0x1))
			pushStepX = 0;
		if (pushStepY > 0 && !(peepEdgeLimits & 0x2))
			pushStepY = 0;
		if (pushStepX > 0 && !(peepEdgeLimits & 0x4))
			pushStepX = 0;
		if (pushStepY < 0 && !(peepEdgeLimits & 0x8))
			pushStepY = 0;
        
        if (scenario_rand_max(3) == 0) {
		    if (actualStepX == 0) {
			    if (expectAsideX > 0)
				    pushStepX += 1;
			    else if (expectAsideX < 0)
				    pushStepX -= 1;
		    }
		    if (actualStepY == 0) {
			    if (expectAsideY > 0)
				    pushStepY += 1;
			    else if (expectAsideY < 0)
				    pushStepY -= 1;
		    }
        }
        
        if (peep->happiness > 60 && peep->peepex_follow_target == 0x0) {
                // Happy peeps will avoid, unhappy peeps will just bump into people
		    peep->destination_x	+= pushStepX;
		    peep->destination_y	+= pushStepY;
        }

		if (actualStepX <= 0 && pushAsideX > 0)
			pushStepX += 1;
		else if (actualStepX >= 0 && pushAsideX < 0)
			pushStepX -= 1;
		if (actualStepY <= 0 && pushAsideY > 0)
			pushStepY += 1;
		else if (actualStepY >= 0 && pushAsideY < 0)
			pushStepY -= 1;
		
		if (pushStepX < 0 && !(peepEdgeLimits & 0x1))
			pushStepX = 0;
		if (pushStepY > 0 && !(peepEdgeLimits & 0x2))
			pushStepY = 0;
		if (pushStepX > 0 && !(peepEdgeLimits & 0x4))
			pushStepX = 0;
		if (pushStepY < 0 && !(peepEdgeLimits & 0x8))
			pushStepY = 0;

        if (scenario_rand_max(peep->energy / 64) == 0) {
            if (bumpAsideX != 0)
                actualStepY -= (0 < actualStepY) - (actualStepY < 0);
            if (bumpAsideY != 0)
                actualStepX -= (0 < actualStepX) - (actualStepX < 0);
        }

	    mapElement = map_get_path_element_at(peep->destination_x / 32, peep->destination_y / 32, peep->z >> 3);
	    if (mapElement) {
            if (mapElement->flags & MAP_ELEMENT_FLAG_TEMPORARILY_BLOCKED) {
	            mapElement = map_get_path_element_at(peep->x / 32, peep->y / 32, peep->z >> 3);
                if (!(mapElement->flags & MAP_ELEMENT_FLAG_TEMPORARILY_BLOCKED)) {
                    actualStepX = sidestepX = 0;
                    actualStepY = sidestepY = 0;
                }
            }
        }
        
        if (actualStepX + sidestepX + pushStepX == 0 &&
              actualStepY + sidestepY + pushStepY == 0) {
            peep->no_action_frame_no = 0;
        }
	}

	*x = peep->x + actualStepX + sidestepX + pushStepX;
	*y = peep->y + actualStepY + sidestepY + pushStepY;
}

sint32 peep_update_queue_position_messy(rct_peep* peep, uint8 previous_action)
{
	rct_peep*	peep_next = GET_PEEP(peep->next_in_queue);
	Ride*		peep_ride = get_ride(peep->current_ride);

	sint16 x_diff = abs(peep_next->x - peep->x);
	sint16 y_diff = abs(peep_next->y - peep->y);
	sint16 z_diff = abs(peep_next->z - peep->z);

	if (z_diff > 30)
 		return 0;

	if (x_diff < y_diff){
		sint16 temp_x = x_diff;
		x_diff = y_diff;
		y_diff = temp_x;
	}

	x_diff += y_diff / 2;

		// At the last tile, it's best not to do any strange things.
		//	this variable checks if there is a tile ahead (will not be
		//	checked more often than once)

	int		prohibitShenanigans	= 1;
	int		peepsAhead			= 0;
	bool	allowMove			= true;

	uint16 macroCurX = peep->x >> 5;
	uint16 macroCurY = peep->y >> 5;
	uint16 macroDesX = peep->destination_x >> 5;
	uint16 macroDesY = peep->destination_y >> 5;
	uint16 macroCurNxX = peep_next->x >> 5;
	uint16 macroCurNxY = peep_next->y >> 5;

	if (peep_next->next_in_queue != 0xFFFF) {
		rct_peep *nxtCue = peep_next;

		while (true) {
			if (nxtCue->next_in_queue == 0xFFFF) {
				break;
			}
			nxtCue = GET_PEEP(nxtCue->next_in_queue);

			peepsAhead += 1;

			uint16 macroCurFarX = nxtCue->x >> 5;
			uint16 macroCurFarY = nxtCue->y >> 5;

			if (macroCurFarX == macroCurNxX &&
				macroCurFarY == macroCurNxY) {
				continue;
			}

				// Someone is ahead of the person blocking us, in another tile;
				//	we could never, ever share that tile!

			if (macroCurX == macroCurFarX &&
				macroCurY == macroCurFarY) {
				allowMove = false;
			}

			if (prohibitShenanigans > 0)
				prohibitShenanigans -= 1;

			while (peepsAhead < 40) {
				if (nxtCue->next_in_queue == 0xFFFF) {
					break;
				}
				nxtCue = GET_PEEP(nxtCue->next_in_queue);
				peepsAhead += 1;
			}

			break;
		}
	}

	// Are we moving? Take up more space! Are we not? Reduce space!
	// Are we and the next both stopped? Nudge hir to queue up better.

	// The most significant bit in peeps_ex_queue_wait_distance contains
	//	whether a peep was moving. This is used to 'propagate' peeps
	//	waiting close-by one-another.

	uint32 tickRef = peep->time_in_queue;

	if (tickRef % 11 == 0) {
		if (peep->peepex_queue_wait_distance & 0x80) {
			if ((peep->peepex_queue_wait_distance & 0x7F) < 14) {
				peep->peepex_queue_wait_distance += scenario_rand_max(6);
				if ((macroCurNxX != macroCurX || macroCurNxY != macroCurY) &&
					(macroCurNxX != macroDesX || macroCurNxY != macroDesY)) {
					peep->peepex_queue_wait_distance += 2;
				}
			}
		}
	}
	if (tickRef % 23 == 0) {
		if (!(peep_next->peepex_queue_wait_distance & 0x80)) {
			if (peep->peepex_queue_wait_distance < peep_next->peepex_queue_wait_distance) {
				peep_next->peepex_queue_wait_distance = max(7, min(peep_next->peepex_queue_wait_distance, peep->peepex_queue_wait_distance + scenario_rand_max(2)));
			}
			if (!(peep->peepex_queue_wait_distance & 0x80)) {
				int randomCount = 2048;
				uint32 randNum = scenario_rand_max(16 * randomCount);
				int customerCheck = 5 + peep_ride->cur_num_customers / 2;
				uint32 decreaseCheck = min(randomCount, (customerCheck - max(customerCheck, peepsAhead) * (randomCount / 16)));
				if ((randNum % randomCount) <= decreaseCheck) {
					peep->peepex_queue_wait_distance -= randNum / randomCount;
					peep->peepex_queue_wait_distance = max(7, peep->peepex_queue_wait_distance);
				}
			}
		}
		if ((peep->peepex_queue_wait_distance & 0x7F) < 6) {
			peep->peepex_queue_wait_distance += 1;
		}
	}

	if (allowMove) {	
		peep_next	= peep;

		for (int ahead = 0; ahead <3; ahead++) {
			if (peep_next->next_in_queue == 0xFFFF) {
				break;
			}
			peep_next = GET_PEEP(peep_next->next_in_queue);

			x_diff = abs(peep_next->x - peep->x);
			y_diff = abs(peep_next->y - peep->y);

			if (x_diff < y_diff) {
				sint16 temp_x = x_diff;
				x_diff = y_diff;
				y_diff = temp_x;
			}

			x_diff += y_diff / 2;

			if (x_diff < (0x7F & peep->peepex_queue_wait_distance)) {
				allowMove = false;
				break;
			}
		}
	}
	if (allowMove) {
			// State that we are moving
		peep->peepex_queue_wait_distance |= 0x80;

		if (prohibitShenanigans == 0) {
			if (macroDesX == peep_next->x >> 5 &&
				macroDesY == peep_next->y >> 5) {

				uint16 tarX = peep_next->x;
				uint16 tarY = peep_next->y;

				if (tickRef % 3 == 0) {
					int maxOffset = 5;
					
					tarX += -maxOffset + scenario_rand_max(2 * maxOffset + 1);
					tarY += -maxOffset + scenario_rand_max(2 * maxOffset + 1);
				}

				if (macroDesX != macroCurX) {
					if (macroDesX > macroCurX) {
						peep->destination_x = max((macroDesX << 5) + 10, tarX);
					}
					else {
						peep->destination_x = min((macroDesX << 5) + 22, tarX);
					}

					peep->destination_y = min(max((macroDesY << 5) + 10, tarY), (macroDesY << 5) + 22);
				}
				else if (macroDesY != macroCurY) {
					if (macroDesY > macroCurY) {
						peep->destination_y = max((macroDesY << 5) + 10, tarY);
					}
					else {
						peep->destination_y = min((macroDesY << 5) + 22, tarY);
					}

					peep->destination_x = min(max((macroDesX << 5) + 10, tarX), (macroDesX << 5) + 22);
				}

				peep->destination_tolerence = 2;
			}
			else {
				sint16 x = peep->destination_x & 0xFFE0;
				sint16 y = peep->destination_y & 0xFFE0;

				sint16 tileOffsetX = peep->destination_x & ~0xFFE0;
				sint16 tileOffsetY = peep->destination_y & ~0xFFE0;

				tileOffsetX -= 16;
				tileOffsetY -= 16;

				tileOffsetX += (peep->x - peep->destination_x) / 3;
				tileOffsetY += (peep->y - peep->destination_y) / 3;

				if (tickRef % 3 == 0) {
					tileOffsetX += -7 + scenario_rand_max(15);
					tileOffsetY += -7 + scenario_rand_max(15);
				}

				sint16 maxTileOffset = 6;

				if (tileOffsetX < -maxTileOffset)
					tileOffsetX = -maxTileOffset;
				else if (tileOffsetX > maxTileOffset)
					tileOffsetX = maxTileOffset;
				if (tileOffsetY < -maxTileOffset)
					tileOffsetY = -maxTileOffset;
				else if (tileOffsetY > maxTileOffset)
					tileOffsetY = maxTileOffset;

				peep->destination_x = x + tileOffsetX + 16;
				peep->destination_y = y + tileOffsetY + 16;

				peep->destination_tolerence = 3;
			}
		}

		return 0;
	}
	else {
			// We are not moving
		peep->peepex_queue_wait_distance &= 0x7F;
 	}

	return 1;
}

sint32 peep_move_one_tile_messy(sint32 x, sint32 y, uint8 direction, rct_peep* peep)
{	
	sint16 tileOffsetX = peep->x & ~0xFFE0;
	sint16 tileOffsetY = peep->y & ~0xFFE0;

	tileOffsetX -= 16;
	tileOffsetY -= 16;

	tileOffsetX -= ((TileDirectionDelta[direction].x) * scenario_rand_max(16)) / 4;
	tileOffsetY -= ((TileDirectionDelta[direction].y) * scenario_rand_max(16)) / 4;

	tileOffsetX += 1 - scenario_rand_max(3);
	tileOffsetY += 1 - scenario_rand_max(3);

	sint32 edges = 0x0;
	
	sint16 peepMacroTileNextX		= x / 32;
	sint16 peepMacroTileNextY		= y / 32;

    rct_map_element *mapElement = map_get_path_element_at(peepMacroTileNextX, peepMacroTileNextY, peep->next_z);
    if (mapElement) {
		sint32 nextEdges = mapElement->properties.path.edges;
		sint32 curEdges = 0;
		
		mapElement = map_get_path_element_at(peep->x / 32, peep->y / 32, peep->next_z);
		if (mapElement)
			curEdges = mapElement->properties.path.edges;

		if ((curEdges & 0x1) == (nextEdges & 0x1))
			edges |= 0xA;
		if ((curEdges & 0xA) == (nextEdges & 0xA))
			edges |= 0x5;
		
		edges &= curEdges;
		edges &= nextEdges;
    }

	//peep->peepex_path_limits = edges;

	if (peep->state == PEEP_STATE_WALKING) {
		sint16 enterOffsetX = 0;
		sint16 enterOffsetY = 0;

		{
			switch (peep->peepex_direction_preference) {
			case 0:
				enterOffsetX = -7 + scenario_rand_max(4);
				break;
			case 8:
				enterOffsetY = 7 + scenario_rand_max(4);
				break;
			case 16:
				enterOffsetX = 7 + scenario_rand_max(4);
				break;
			case 24:
				enterOffsetY = -7 + scenario_rand_max(4);
				break;
			default:
				break;
			};
		}

        tileOffsetX += enterOffsetX;
        tileOffsetY += enterOffsetY;

		int offset = 1 + min(10, max(0, (peep->nausea / 50) - 3) + (peep->peepex_crowded_store / 20));

		tileOffsetX += -offset + scenario_rand_max(1 + (offset << 1));
		tileOffsetY += -offset + scenario_rand_max(1 + (offset << 1));

		sint16 baseTileOffset = 6;
		sint16 maxTileOffset[4] = { -baseTileOffset, baseTileOffset, baseTileOffset, -baseTileOffset };
		
		if (edges & 0x1)
			maxTileOffset[0] = -20;
		if (edges & 0x2)
			maxTileOffset[1] = 20;
		if (edges & 0x4)
			maxTileOffset[2] = 20;
		if (edges & 0x8)
			maxTileOffset[3] = -20;

		if (tileOffsetX < maxTileOffset[0])
			tileOffsetX = maxTileOffset[0];
		else if (tileOffsetX > maxTileOffset[2])
			tileOffsetX = maxTileOffset[2];
		if (tileOffsetY < maxTileOffset[3])
			tileOffsetY = maxTileOffset[3];
		else if (tileOffsetY > maxTileOffset[1])
			tileOffsetY = maxTileOffset[1];

		switch (direction) {
		case 0:
			if (tileOffsetX > 14)
				tileOffsetX = 14;
			break;
		case 1:
			if (tileOffsetY < -14)
				tileOffsetY = -14;
			break;
		case 2:
			if (tileOffsetX < -14)
				tileOffsetX = -14;
			break;
		case 3:
			if (tileOffsetY > 14)
				tileOffsetY = 14;
			break;
		}
	}
	else {
		tileOffsetX = 0;
		tileOffsetY = 0;
	}

	peep->direction = direction;
	peep->destination_x = x + tileOffsetX + 16;
	peep->destination_y = y + tileOffsetY + 16;
	peep->destination_tolerence = 2;
	if (peep->state != PEEP_STATE_QUEUING) {
		peep->destination_tolerence = 2;
	}
	else {
		peep->destination_tolerence = 2;
	}

	return 0;
}

void peepex_entertainer_per_tile(rct_peep *peep)
{
    if (gConfigPeepEx.enable_hamelin_entertainer) {
        peepex_event_broadcast_instr e = create_peepex_event_broadcast_instr();
        e.broadcast_type = PEEPEX_BROADCAST_EVENT_GENERIC_VISUAL_ODDITY;
        e.primary_peep   = peep;
        peepex_broadcast_event(&e);
    }
}

void peepex_entertainer_does_event(rct_peep *peep)
{
    if (gConfigPeepEx.enable_hamelin_entertainer) {
        peepex_event_broadcast_instr e = create_peepex_event_broadcast_instr();
        e.broadcast_type = PEEPEX_BROADCAST_EVENT_HAMELIN_DISPLAY;
        e.primary_peep   = peep;
        peepex_broadcast_event(&e);
    }
}

void peepex_prepare_range(peepex_find_peep_in_range_instr *instr)
{
    uint32 foundCount = sprite_get_first_in_multiple_quadrants( instr->point.x - instr->range, instr->point.y - instr->range,
                                                                instr->point.x + instr->range, instr->point.y + instr->range, instr->temp_buffer, instr->max_peeps );
    if (foundCount < instr->max_peeps)
        instr->temp_buffer[foundCount] = SPRITE_INDEX_NULL;

    instr->reading_point = 0;
}

rct_peep *peepex_get_next_peep(peepex_find_peep_in_range_instr *instr, rct_xyz16 *xyz_direction, rct_peep *previous)
{
    uint16 nextSprite = SPRITE_INDEX_NULL;
    rct_sprite *sprite;
    
    if (previous)
        nextSprite = previous->next_in_quadrant;

    while (true) {
        if (nextSprite == SPRITE_INDEX_NULL) {
            nextSprite = instr->temp_buffer[instr->reading_point++];
            if (nextSprite == SPRITE_INDEX_NULL)
                return 0;
        }
        
	    sprite = get_sprite(nextSprite);
        nextSprite = sprite->unknown.next_in_quadrant;

        if (sprite->unknown.sprite_identifier != SPRITE_IDENTIFIER_PEEP)
            continue;
        
		rct_peep* other_peep = (rct_peep*)sprite;

		xyz_direction->x = other_peep->x - instr->point.x;
		xyz_direction->y = other_peep->y - instr->point.y;
		xyz_direction->z = other_peep->z - instr->point.z;

        if ((xyz_direction->x*xyz_direction->x + xyz_direction->y*xyz_direction->y) > (instr->range*instr->range))
            continue;

        return other_peep;
    }
}