#include "peepex.h"
#include "staff.h"
#include "../ride/ride.h"
#include "../world/footpath.h"
#include "../world/entrance.h"
#include "../scenario/scenario.h"
#include "../config/Config.h"
#include "../audio/audio.h"

uint32 peepex_get_appropriate_step_count(rct_peep *peep)
{
    uint32 stepsToTake = peep->energy;

    if (stepsToTake < 95 && peep->state == PEEP_STATE_QUEUING)
        stepsToTake = 95;
    if ((peep->peep_flags & PEEP_FLAGS_SLOW_WALK) && peep->state != PEEP_STATE_QUEUING)
        stepsToTake /= 2;
    if (peep->action == 255 && (peep->next_var_29 & 4)) {
        stepsToTake /= 2;
        if (peep->state == PEEP_STATE_QUEUING)
            stepsToTake += stepsToTake / 2;
    }

	if (gConfigPeepEx.enable_messy_congestion && peep->state == PEEP_STATE_WALKING) {
		stepsToTake *= 4;
		stepsToTake /= 5;
		if (peep->peepex_crowded_store > 10) {
			stepsToTake /= 1 + scenario_rand_max(2);
		}
		else if (peep->peepex_crowded_store > 7) {
			stepsToTake *= 6;
			stepsToTake /= 4 + scenario_rand_max(2);
		}
	}

        // If we are chasing a peep, go very fast
    if (peep->state == PEEP_STATE_EX_SECURITY_CHASING) {
        stepsToTake = 200;
    }
    else if (peep->state == PEEP_STATE_EX_SECURITY_ESCORTING_OUT ||
             peep->state == PEEP_STATE_EX_ESCORTED_BY_STAFF) {
        stepsToTake = 100;
    }

    return stepsToTake;
}

uint8 peepex_effective_peep_interest_in_generic_events(rct_peep *peep)
{
        // these peeps would not be available for activities
    if (peep->state != PEEP_STATE_WALKING &&
        peep->state != PEEP_STATE_PATROLLING)
        return 0;

        // this peep is on a cooldown
    if (peep->peepex_event_countdown > 0)
        return 0;
    
    return (peep->peepex_interest_in_misc & 0xF) << 3;
}

uint8 peepex_effective_peep_interest_in_entertainers(rct_peep *peep)
{
        // these peeps would not be available for activities
    if (peep->state != PEEP_STATE_WALKING &&
        peep->state != PEEP_STATE_PATROLLING)
        return 0;
    
    uint32 interest = (peep->peepex_interest_in_misc >> 4);
    interest *= 8 + interest / 2;
    return min(interest, 0xFF);
}

uint8 peepex_effective_peep_interest_in_generic_rides(rct_peep *peep)
{
        // these peeps would not be available for activities
    if (peep->state != PEEP_STATE_WALKING &&
        peep->state != PEEP_STATE_PATROLLING)
        return 0;

        // this peep is on a cooldown
    if (peep->peepex_event_countdown > 0)
        return 0;
    
    uint32 interest = (peep->peepex_interest_in_rides & 0xF);
    interest *= interest;
    interest >>= 4;
    interest *= (8 + interest / 2) * 2;
    return min(interest, 0xFF);
}

uint8 peepex_effective_peep_interest_in_exciting_rides(rct_peep *peep)
{
        // these peeps would not be available for activities
    if (peep->state != PEEP_STATE_WALKING &&
        peep->state != PEEP_STATE_PATROLLING)
        return 0;

        // this peep is on a cooldown
    if (peep->peepex_event_countdown > 0)
        return 0;
    
    uint32 interest = (peep->peepex_interest_in_rides >> 4);
    interest *= 8 + interest / 2;
    return min(interest, 0xFF);
}

uint8 peepex_effective_peep_interest_in_theft(rct_peep *peep)
{
        // these peeps would not be available for activities
    if (peep->state != PEEP_STATE_WALKING &&
        peep->state != PEEP_STATE_PATROLLING)
        return 0;

        // this peep is on a cooldown
    if (peep->peepex_event_countdown > 0)
        return 0;
    
    uint32 interest = (peep->peepex_interest_in_rides >> 4);
    interest *= 8 + interest / 2;
    return min(interest, 0xFF);
}