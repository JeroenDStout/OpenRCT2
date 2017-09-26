#include "peepex.h"
#include "staff.h"
#include "../ride/ride.h"
#include "../world/footpath.h"
#include "../world/entrance.h"
#include "../scenario/scenario.h"
#include "../config/Config.h"
#include "../audio/audio.h"

void peepex_broadcast_event(peepex_event_broadcast_instr *instr)
{
    switch (instr->broadcast_type) {
    case PEEPEX_BROADCAST_EVENT_GENERIC_ODDITY:
        peepex_broadcast_event_generic_oddity(instr, true, true);
        break;
    case PEEPEX_BROADCAST_EVENT_GENERIC_VISUAL_ODDITY:
        peepex_broadcast_event_generic_oddity(instr, true, false);
        break;
    case PEEPEX_BROADCAST_EVENT_GENERIC_AUDIO_ODDITY:
        peepex_broadcast_event_generic_oddity(instr, false, true);
        break;
    case PEEPEX_BROADCAST_EVENT_HAMELIN_DISPLAY:
        peepex_broadcast_event_hamelin_display(instr);
        break;
    case PEEPEX_BROADCAST_EVENT_HAMELIN_SNARE:
        peepex_broadcast_event_hamelin_snare(instr);
        break;
    }
}

bool peepex_check_peep_notice_thing(rct_peep *peep, rct_xyz16 direction, sint16 distancePow, sint16 visualNear, uint16 visualOdds, sint16 audioNear, uint16 audioOdds)
{
        // we heard it
    if (distancePow < audioNear)
        return true;

        // are we looking at the performer?
    uint8 targetDir = peepex_direction_from_xy(direction.x, direction.y);
    if (targetDir == peep->sprite_direction) {
            // are we really close?
        if (distancePow < visualNear)
            return true;
        if (visualOdds > 0 && scenario_rand_max(visualOdds) == 0)
            return true;
    }
    
    if (audioOdds > 0 && scenario_rand_max(audioOdds) == 0)
        return true;

    return false;
}

void peepex_broadcast_event_generic_oddity(peepex_event_broadcast_instr *instr, bool visual, bool audio)
{
    uint16 peepBuffer[256];
    rct_peep *curPeep = 0;

    peepex_find_peep_in_range_instr findPeeps;
    findPeeps.point.x = instr->primary_peep->x;
    findPeeps.point.y = instr->primary_peep->y;
    findPeeps.point.z = instr->primary_peep->z;
    findPeeps.temp_buffer = peepBuffer;
    findPeeps.max_peeps = 256;
    findPeeps.range = 100;
    peepex_prepare_range(&findPeeps);

    rct_xyz16 xyz_direction;
    while (true) {
        curPeep = peepex_get_next_peep(&findPeeps, &xyz_direction, curPeep);
        if (!curPeep)
            break;

        if (curPeep == instr->primary_peep)
            continue;

        uint8 interest = peepex_effective_peep_interest_in_generic_events(curPeep);
        if (interest == 0)
            continue;
        
        sint16 distance = (xyz_direction.x*xyz_direction.x) + (xyz_direction.y*xyz_direction.y);
        if (!peepex_check_peep_notice_thing(curPeep, xyz_direction, distance, 128, visual? 8 : 0, 40, audio? 4 : 0))
            continue;

        if (scenario_rand_max(0xFF) > interest)
            continue;

        log_warning("Hamelin from seeing one");
        peepex_make_witness(curPeep, instr->primary_peep->sprite_index);
    }
}

void peepex_broadcast_event_hamelin_display(peepex_event_broadcast_instr *instr)
{
    uint16 peepBuffer[256];
    rct_peep *curPeep = 0;

    peepex_find_peep_in_range_instr findPeeps;
    findPeeps.point.x = instr->primary_peep->x;
    findPeeps.point.y = instr->primary_peep->y;
    findPeeps.point.z = instr->primary_peep->z;
    findPeeps.temp_buffer = peepBuffer;
    findPeeps.max_peeps = 256;
    findPeeps.range = 100;
    peepex_prepare_range(&findPeeps);

    rct_xyz16 xyz_direction;
    while (true) {
        curPeep = peepex_get_next_peep(&findPeeps, &xyz_direction, curPeep);
        if (!curPeep)
            break;

        if (curPeep == instr->primary_peep)
            continue;

        uint8 interest = peepex_effective_peep_interest_in_entertainers(curPeep);
        if (interest == 0)
            continue;
        
        sint16 distance = (xyz_direction.x*xyz_direction.x) + (xyz_direction.y*xyz_direction.y);
        if (!peepex_check_peep_notice_thing(curPeep, xyz_direction, distance, 128, 8, 64, 4))
            continue;

        if (scenario_rand_max(0xFF) > interest)
            continue;

        log_warning("Hamelin from seeing one");
        peepex_make_hamelin(curPeep, instr->primary_peep);
    }
}

void peepex_broadcast_event_hamelin_snare(peepex_event_broadcast_instr *instr)
{
    uint16 peepBuffer[256];
    rct_peep *curPeep = 0;

    peepex_find_peep_in_range_instr findPeeps;
    findPeeps.point.x = instr->primary_peep->x;
    findPeeps.point.y = instr->primary_peep->y;
    findPeeps.point.z = instr->primary_peep->z;
    findPeeps.temp_buffer = peepBuffer;
    findPeeps.max_peeps = 256;
    findPeeps.range = 100;
    peepex_prepare_range(&findPeeps);

    rct_xyz16 xyz_direction;
    while (true) {
        curPeep = peepex_get_next_peep(&findPeeps, &xyz_direction, curPeep);
        if (!curPeep)
            break;

        if (curPeep == instr->primary_peep)
            continue;

        uint8 interest = peepex_effective_peep_interest_in_entertainers(curPeep);
        if (interest == 0)
            continue;
        
        sint16 distance = (xyz_direction.x*xyz_direction.x) + (xyz_direction.y*xyz_direction.y);
        if (!peepex_check_peep_notice_thing(curPeep, xyz_direction, distance, 128, 0, 64, 16))
            continue;

        if (scenario_rand_max(0xFF) > interest)
            continue;
        
        if (scenario_rand_max(4) == 0) {
            log_warning("Witness from snare");
            peepex_make_witness(curPeep, instr->primary_peep->sprite_index);
        }
        else {
            log_warning("Hamelin from snare");
            peepex_make_hamelin(curPeep, GET_PEEP(instr->primary_peep->peepex_follow_target));
        }
    }
}

peepex_event_broadcast_instr create_peepex_event_broadcast_instr()
{
    peepex_event_broadcast_instr  instr;

    instr.broadcast_type    = PEEPEX_BROADCAST_EVENT_NONE;
    instr.broadcast_mode    = PEEPEX_BROADCAST_MODE_NORMAL;

    return instr;
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

    return 0x80;
}

uint8 peepex_effective_peep_interest_in_entertainers(rct_peep *peep)
{
        // these peeps would not be available for activities
    if (peep->state != PEEP_STATE_WALKING &&
        peep->state != PEEP_STATE_PATROLLING)
        return 0;

        // this peep is on a cooldown
    if (peep->peepex_hamelin_countdown > 0)
        return 0;

    return 0x80;
}