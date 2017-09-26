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

#ifndef _PEEPEX_H_
#define _PEEPEX_H_

#include "peep.h"

enum {
    PEEPEX_FOLLOWING_FLAG_FREE_WALK     = 1 << 0,
    PEEPEX_FOLLOWING_FLAG_NOSY          = 2 << 0        // the peep will be more persistent
};

typedef struct peepex_follow_instr {
    uint16    attempt_min_distance;
    uint16    attempt_max_distance;
    uint16    target_forward_offset;
    sint16    base_gradient_weight_in_percent;
    sint16    crowd_weight_in_percent;

    bool      out_target_lost;
    bool      out_comfortable_position;
    uint16    out_effective_distance;
    uint8     out_facing_direction;
} peepex_follow_instr;
peepex_follow_instr create_peepex_follow_instr();

typedef struct peepex_sliding_check_instr {
    rct_peep  *peep;
    rct_xyz16 current, target;
    sint16    max_xy_distance, max_z_distance;

    bool      out_target_is_reachable;
} peepex_sliding_check_instr;

typedef struct peepex_pathing_hint_instr {
    rct_xyz16 current, target;
    uint8     max_search_depth;

    bool      out_found_target;
    uint8     out_primary_direction;
} peepex_pathing_hint_instr;

typedef struct peepex_event_broadcast_instr {
    uint8     broadcast_type;
    uint8     broadcast_mode;
    rct_peep* primary_peep;
} peepex_event_broadcast_instr;
peepex_event_broadcast_instr create_peepex_event_broadcast_instr();

typedef struct peepex_find_peep_in_range_instr {
    rct_xyz16  point;
    uint16*    temp_buffer;
    uint32     max_peeps;
    sint32     range;
    uint32     reading_point;
} peepex_find_peep_in_range_instr;
void peepex_prepare_range(peepex_find_peep_in_range_instr*);
rct_peep *peepex_get_next_peep(peepex_find_peep_in_range_instr*, rct_xyz16 *xyz_direction, rct_peep *previous);

enum {
    PEEPEX_BROADCAST_EVENT_NONE                   = 0,
    PEEPEX_BROADCAST_EVENT_GENERIC_ODDITY,
    PEEPEX_BROADCAST_EVENT_GENERIC_VISUAL_ODDITY,
    PEEPEX_BROADCAST_EVENT_GENERIC_AUDIO_ODDITY,
    PEEPEX_BROADCAST_EVENT_HAMELIN_DISPLAY,
    PEEPEX_BROADCAST_EVENT_HAMELIN_SNARE
};

enum {
    PEEPEX_BROADCAST_MODE_NORMAL            = 0
};

void peepex_base_update(rct_peep *peep, sint32 index);
void peep_update_action_sidestepping(sint16* x, sint16* y, sint16 x_delta, sint16 y_delta, sint16* xy_distance, rct_peep* peep);
sint32 peep_update_queue_position_messy(rct_peep* peep, uint8 previous_action);
sint32 peep_move_one_tile_messy(sint32 x, sint32 y, uint8 direction, rct_peep* peep);
void peepex_update_following(rct_peep *peep, peepex_follow_instr* instr);
void peepex_sliding_check(peepex_sliding_check_instr* instr);
void peepex_pathing_hint(peepex_pathing_hint_instr* instr);
bool peepex_find_connected_path(rct_peep*, rct_map_element*, sint16 x, sint16 y, uint8 direction, sint16 *nextZ);
uint8 peepex_direction_from_xy(sint16 x, sint16 y);

bool peepex_check_peep_notice_thing(rct_peep *peep, rct_xyz16 direction, sint16 distancePow, sint16 visualNear, uint16 visualOdds, sint16 audioNear, uint16 audioOdds);

    // update 
bool peepex_update_walking_find_activity(rct_peep *peep);
bool peepex_update_walking_find_activity_hamelin(rct_peep *peep, uint16 otherPeep);
bool peepex_update_patrolling_find_activity(rct_peep *peep);

    // make do
void peepex_make_witness(rct_peep *peep, uint16 target);
void peepex_make_hamelin(rct_peep *peep, rct_peep *hamelin);
void peepex_make_security_escort_out(rct_peep *peep);
void peepex_return_to_walking(rct_peep *peep);

    // events
void peepex_broadcast_event(peepex_event_broadcast_instr*);
void peepex_broadcast_event_generic_oddity(peepex_event_broadcast_instr*, bool visual, bool audio);
void peepex_broadcast_event_hamelin_display(peepex_event_broadcast_instr*);
void peepex_broadcast_event_hamelin_snare(peepex_event_broadcast_instr*);

    // new behaviours
void peepex_update_witness(rct_peep *peep);
void peepex_update_hamelin(rct_peep *peep);
void peepex_update_escorted_by_staff(rct_peep *peep);
void peepex_update_security_chasing(rct_peep *peep);
void peepex_update_security_escorting_out(rct_peep *peep);

    // entertainers
void peepex_entertainer_per_tile(rct_peep *peep);
void peepex_entertainer_does_event(rct_peep *peep);

    // interests
uint8 peepex_effective_peep_interest_in_generic_events(rct_peep *peep);
uint8 peepex_effective_peep_interest_in_entertainers(rct_peep *peep);

#endif