
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <math.h>


#include "sim_drv.h"
#include "wd_of_ants.h"


/* Chain of simple drivers. */
static sim_drv * drivers = 0l;

sim_drv * all_simple_drivers( void )
{
	return drivers;
}


void add_sim_drv( wd_of_ants * world, ant * c_ant )
{
	sim_drv * new = ( sim_drv * )malloc( sizeof( struct sim_drv ) );
	memset( new, 0, sizeof( struct sim_drv ) );
	assert( new );
	
	new->the_ant = c_ant;
	new->world = world;

	/* Here we must init all data structures. */

	new->next = drivers;
	drivers = new;
}


static void make_sp3_seg( sp3_seg * p, float beg_x, float beg_y, float beg_ang,
					float end_x, float end_y, float end_ang )
{
	/* Polynome Inputs. */
	p->begin_x = beg_x;
	p->begin_y = beg_y;
	p->begin_dx = cos( beg_ang );
	p->begin_dy = sin( beg_ang );

	p->end_x = end_x;
	p->end_y = end_y;
	p->end_dx = cos( end_ang );
	p->end_dy = sin( end_ang );

	/* Polynome. */
	p->bx[0] = p->begin_x;
	p->bx[1] = p->begin_dx;
	p->bx[2] = 3 * ( p->end_x - p->begin_x ) - 2 * p->begin_dx - p->end_dx;
	p->bx[3] = 2 * ( p->begin_x - p->end_x ) + p->begin_dx + p->end_dx;	

	p->by[0] = p->begin_y;
	p->by[1] = p->begin_dy;
	p->by[2] = 3 * ( p->end_y - p->begin_y ) - 2 * p->begin_dy - p->end_dy;
	p->by[3] = 2 * ( p->begin_y - p->end_y ) + p->begin_dy + p->end_dy;	
}


void exec_sim_drv( void )
{
	sim_drv * drv = drivers;

	while( drv )
	{
		/* Check for task. */
static int cc = 0; cc++;
		if( ( !drv->act_task.status ) || !( cc % 500 ) )
		{
			get_next_task( &( drv->act_task ), 
					drv->the_ant, drv->world );

			make_sp3_seg( &( drv->trace ), 
				drv->the_ant->pos_x, drv->the_ant->pos_y, drv->the_ant->pos_ang,
				drv->act_task.tg_x, drv->act_task.tg_y, drv->act_task.tg_ang );
		}

		/* Test route */

		/* Rule */
		
		drv = drv->next;
	}
}


void close_sim_drv( void )
{
	sim_drv * cur_drv = drivers;

	while( cur_drv )
	{
		sim_drv * t_drv = cur_drv;

		cur_drv = cur_drv->next;
		free( t_drv );
	}
}

