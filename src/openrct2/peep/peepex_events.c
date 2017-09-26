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
        peepex_broadcast_event_hamelin_snare(instr, true, true);
        break;
    case PEEPEX_BROADCAST_EVENT_HAMELIN_SNARE_AUDIO:
        peepex_broadcast_event_hamelin_snare(instr, true, false);
        break;
    case PEEPEX_BROADCAST_EVENT_HAMELIN_SNARE_VISUAL:
        peepex_broadcast_event_hamelin_snare(instr, false, true);
        break;
    }

    peepex_broadcast_show_sprite_hint(instr);
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

void peepex_broadcast_event_hamelin_snare(peepex_event_broadcast_instr *instr, bool visual, bool audio)
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
        if (!peepex_check_peep_notice_thing(curPeep, xyz_direction, distance, visual? 128 : 0, 0, 64, audio? 4 : 0))
            continue;

        if (scenario_rand_max(0xFF) > interest)
            continue;
        
        if (scenario_rand_max(4) == 0) {
            peepex_make_witness(curPeep, instr->primary_peep->sprite_index);
        }
        else {
            if (scenario_rand_max(0xFF) > peepex_effective_peep_interest_in_entertainers(curPeep))
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
    
    return (peep->peepex_interest_in_misc & 0xF) << 4;
}

uint8 peepex_effective_peep_interest_in_entertainers(rct_peep *peep)
{
        // these peeps would not be available for activities
    if (peep->state != PEEP_STATE_WALKING &&
        peep->state != PEEP_STATE_PATROLLING)
        return 0;

    return peep->peepex_interest_in_misc & 0xF0;
}

void peepex_update_interest(rct_peep *peep)
{
        // Every 10 seconds, increase our interest again. This way,
        //  we can reach a max of 15 interest in 2.5 minutes.
        // However, peeps are limited in their final interest by
        //  a deterministic value generated by their sprite id.
    
        // TODO: elegantly handle save games without interests
    if (peep->peepex_interest_in_misc == 0)
        peep->peepex_interest_in_misc = peep->sprite_index & 0xFF;

    if (0 == ((gScenarioTicks + peep->sprite_index) % 400)) {
        uint8 maxInterestInEntertainers = (peep->sprite_index & 0xF);
        uint8 maxInterestInEvents       = (peep->sprite_index & 0xF) ^ ((peep->sprite_index & 0xF0) >> 8);


        if ((peep->peepex_interest_in_misc & 0xF0) < (maxInterestInEntertainers << 4))
            peep->peepex_interest_in_misc += 0x10;
        if ((peep->peepex_interest_in_misc & 0x0F) < maxInterestInEvents)
            peep->peepex_interest_in_misc += 0x01;
    }
}

void peepex_broadcast_show_sprite_hint(peepex_event_broadcast_instr *instr)
{
    rct_unk_sprite *sprite;

    rct_xyz16 location;
    if (instr->primary_peep) {
        location.x = instr->primary_peep->x;
        location.y = instr->primary_peep->y;
        location.z = instr->primary_peep->z;
    }
    else {
        return;
    }

    switch (instr->broadcast_type) {
    default:
    case PEEPEX_BROADCAST_EVENT_GENERIC_ODDITY:
    case PEEPEX_BROADCAST_EVENT_GENERIC_VISUAL_ODDITY:
    case PEEPEX_BROADCAST_EVENT_GENERIC_AUDIO_ODDITY:
    case PEEPEX_BROADCAST_EVENT_HAMELIN_DISPLAY:
    case PEEPEX_BROADCAST_EVENT_HAMELIN_SNARE:
        sprite = (rct_unk_sprite*)create_sprite(2);
        if (!sprite) {
            log_warning("NO SPRITE FOR HINT");
            return;
        }
        sprite->sprite_width = 20;
        sprite->sprite_height_negative = 18;
        sprite->sprite_height_positive = 16;
        sprite->sprite_identifier = SPRITE_IDENTIFIER_MISC;
        sprite->misc_identifier = SPRITE_MISC_STEAM_PARTICLE;
        sprite_move(location.x, location.y, location.z, (rct_sprite*)sprite);
    };
}