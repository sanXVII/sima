
#include <stdio.h>
#include <math.h>

#include "ant_furer.h"
#include "wd_of_ants.h"
#include "rtree.h"




int get_next_task( task * pt, ant * pa, wd_of_ants * wd, int not_completed )
{
	free_pix * bpix = 0l;	
	if( wd->free_pixs )
	{
		bpix = booking_free_pix( wd->free_pixs, 0.0, 0.0,
                        100.0/* m */, 1.0/* R */, 0.0/* G */, 0.0/* B */, 0/* free */ );
	}

	if( bpix )
	{
		pt->tg_x = bpix->x - cos( bpix->angle ) * 0.1/* nose */;
		pt->tg_y = bpix->y - sin( bpix->angle ) * 0.1;
		pt->tg_ang = bpix->angle;
	}
	else
	{
		do
		{
        		pt->tg_x = ( float )( rand() % 100 ) / 25.0 - 2.0;
        		pt->tg_y = ( float )( rand() % 100 ) / 25.0 - 2.0;
        		pt->tg_ang = ( float )( rand() % 100 ) / 30.0;
		}
		while( get_next_near( wd->stub->adam, pt->tg_x, pt->tg_y, 0.50/* 30sm */ ) );
	}


	pt->dst_pix = wd->plan.pixs + wd->plan.last_tasked;
	pt->dst_x = wd->plan.left_up_x 
		+ ( wd->plan.last_tasked % wd->plan.width ) * wd->plan.pix_side;
	pt->dst_y = wd->plan.left_up_y
		- ( wd->plan.last_tasked / wd->plan.width ) * wd->plan.pix_side - 0.1/* nose */;
	pt->dst_ang = M_PI/2.0;

	wd->plan.last_tasked++;

        return 0;
}

