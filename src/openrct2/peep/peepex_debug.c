#include "peepex.h"

#define DEBUG_COLOUR_CODE_BEHAVIOUR     1
#define DEBUG_COLOUR_CODE_CRIME         0

void peepex_debug_colour_code(rct_peep *peep)
{
#if DEBUG_COLOUR_CODE_BEHAVIOUR

    switch (peep->state) {
    case PEEP_STATE_EX_WITNESSING_EVENT:
        peep->tshirt_colour = peep->tshirt_colour = 10;
        break;
    case PEEP_STATE_EX_FOLLOWING_HAMELIN:
        peep->tshirt_colour = peep->tshirt_colour = 20;
        break;
    case PEEP_STATE_EX_WATCHING_RIDE:
        peepex_update_watching_ride_cont(peep);
        peep->tshirt_colour = peep->tshirt_colour = 30;
        break;
    default:
        peep->tshirt_colour = peep->tshirt_colour = 0;
    };

#endif

#if DEBUG_COLOUR_CODE_CRIME



#endif
}