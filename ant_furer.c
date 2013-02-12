
#include <stdio.h>

#include "ant_furer.h"
#include "wd_of_ants.h"



int get_next_task( task * pt, ant * pa, wd_of_ants * wd )
{
        pt->status = 1/* new */;
        pt->tg_x = ( float )( rand() % 100 ) / 50.0;
        pt->tg_y = ( float )( rand() % 100 ) / 50.0;
        pt->tg_ang = ( float )( rand() % 100 ) / 30.0;


printf( "tg_x=%f  tg_y=%f  tg_ang=%f\n", pt->tg_x, pt->tg_y, pt->tg_ang );

        return 0;
}

