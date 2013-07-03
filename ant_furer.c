
#include <stdio.h>
#include <math.h>

#include "ant_furer.h"
#include "wd_of_ants.h"
#include "rtree.h"




int get_next_task( task * pt, ant * pa, wd_of_ants * wd, int not_completed )
{
	free_pix * bpix = 0l;
	if( !wd->free_pixs ) return -1;
	if( wd->plan.width * wd->plan.hight <= wd->plan.last_tasked ) return -1;


	pt->dst_pix = wd->plan.pixs + wd->plan.last_tasked;
	pt->dst_x = wd->plan.left_up_x 
		+ ( wd->plan.last_tasked % wd->plan.width ) * wd->plan.pix_side;
	pt->dst_y = wd->plan.left_up_y
		- ( wd->plan.last_tasked / wd->plan.width ) * wd->plan.pix_side - 0.1/* nose */;
	pt->dst_ang = M_PI/2.0;
	
	bpix = booking_free_pix( wd->free_pixs, 0.0, 0.0,
                   100.0/* m */, pt->dst_pix->red, pt->dst_pix->green, pt->dst_pix->blue );

	if( !bpix ) return -1;

	pt->tg_x = bpix->x - cos( bpix->angle ) * 0.1/* nose */;
	pt->tg_y = bpix->y - sin( bpix->angle ) * 0.1;
	pt->tg_ang = bpix->angle;

	pt->tg_r = bpix->red;
	pt->tg_g = bpix->green;
	pt->tg_b = bpix->blue;



	wd->plan.last_tasked++;

        return 0;
}

