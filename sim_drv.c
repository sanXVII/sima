
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include <stdio.h>


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
		if( ( !drv->act_task.status ) || ( drv->now_t >= 1.0 ) )
		{
			get_next_task( &( drv->act_task ), 
					drv->the_ant, drv->world );

			make_sp3_seg( &( drv->trace ), 
				drv->the_ant->pos_x, drv->the_ant->pos_y, drv->the_ant->pos_ang,
				drv->act_task.tg_x, drv->act_task.tg_y, drv->act_task.tg_ang );
			drv->now_t = 0.0; /* begin o spline */
		}

		/* Test route */


		/* Find the error vector. */
		float ex = drv->trace.bx[0] + drv->trace.bx[1] * drv->now_t +
				drv->trace.bx[2] * drv->now_t * drv->now_t + 
				drv->trace.bx[3] * drv->now_t * drv->now_t * drv->now_t;
		float ey = drv->trace.by[0] + drv->trace.by[1] * drv->now_t +
				drv->trace.by[2] * drv->now_t * drv->now_t + 
				drv->trace.by[3] * drv->now_t * drv->now_t * drv->now_t;
		ex -= drv->the_ant->pos_x;
		ey -= drv->the_ant->pos_y;

		drv->the_ant->corr_turn = ex * sin( drv->the_ant->pos_ang * (-1) ) +  
				ey * cos( drv->the_ant->pos_ang * (-1) );
		float mov = ex * cos( drv->the_ant->pos_ang * (-1) ) -
				ey * sin( drv->the_ant->pos_ang * (-1) );
		//mov -= 0.01; /* */
		
		/* Rule. */
		drv->the_ant->left_speed = mov * 400.0;
		drv->the_ant->right_speed = mov * 400.0;

		if( mov > 0.0 )
		{
			drv->the_ant->left_speed -= drv->the_ant->corr_turn * 400.0;
			drv->the_ant->right_speed += drv->the_ant->corr_turn * 400.0;
		}
		else
		{
			drv->the_ant->left_speed += drv->the_ant->corr_turn * 400.0;
			drv->the_ant->right_speed -= drv->the_ant->corr_turn * 400.0;
		}


		/* Move out */
		drv->now_t += 0.003;
		
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

