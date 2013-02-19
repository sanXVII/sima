
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
		float time_plus = 0.002;

		/* Check for task. */
		if( ( !drv->act_task.status ) || ( drv->now_t + time_plus > 1.0 ) )
		{
			get_next_task( &( drv->act_task ), 
					drv->the_ant, drv->world );

			make_sp3_seg( &( drv->trace ), 
				drv->the_ant->pos_x, drv->the_ant->pos_y, drv->the_ant->pos_ang,
				drv->act_task.tg_x, drv->act_task.tg_y, drv->act_task.tg_ang );
			drv->now_t = 0.0; /* begin o spline */
		}

		/* Test route */


		/* Find compensation of error. */
		float now_x = drv->trace.bx[0] + drv->trace.bx[1] * drv->now_t +
				drv->trace.bx[2] * drv->now_t * drv->now_t + 
				drv->trace.bx[3] * drv->now_t * drv->now_t * drv->now_t;
		float now_y = drv->trace.by[0] + drv->trace.by[1] * drv->now_t +
				drv->trace.by[2] * drv->now_t * drv->now_t + 
				drv->trace.by[3] * drv->now_t * drv->now_t * drv->now_t;

		float ex = now_x - drv->the_ant->pos_x;
		float ey = now_y - drv->the_ant->pos_y;

		float corr_turn = ex * sin( drv->the_ant->pos_ang * (-1) ) +  
				ey * cos( drv->the_ant->pos_ang * (-1) );
		float mov = ex * cos( drv->the_ant->pos_ang * (-1) ) -
				ey * sin( drv->the_ant->pos_ang * (-1) );

		corr_turn *= corr_turn;
		mov *= mov;
		
		/* Correction of trajectory. */
		drv->the_ant->left_speed = mov * 2000.0;
		drv->the_ant->right_speed = mov * 2000.0;

		drv->the_ant->left_speed -= corr_turn * 2000.0;
		drv->the_ant->right_speed += corr_turn * 2000.0;

printf( "Correction control: left_w=%f .. right_w=%f\n", drv->the_ant->left_speed, drv->the_ant->right_speed );

		/* Major control. */
		float now_dx = drv->trace.bx[1] + 2 * drv->trace.bx[2] * drv->now_t + 
			3 * drv->trace.bx[3] * drv->now_t * drv->now_t;

		float now_dy = drv->trace.by[1] + 2 * drv->trace.by[2] * drv->now_t +
			3 * drv->trace.by[3] * drv->now_t * drv->now_t;

		float now_norm = sqrt( now_dx * now_dx + now_dy * now_dy );
		now_norm = 1.0 / now_norm;
		
		now_dx *= now_norm * ( drv->the_ant->axis_len / 2.0 );
		now_dy *= now_norm * ( drv->the_ant->axis_len / 2.0 );
		
		drv->now_t += time_plus; /* Move out */

		float next_x = drv->trace.bx[0] + drv->trace.bx[1] * drv->now_t +
				drv->trace.bx[2] * drv->now_t * drv->now_t + 
				drv->trace.bx[3] * drv->now_t * drv->now_t * drv->now_t;
		float next_y = drv->trace.by[0] + drv->trace.by[1] * drv->now_t +
				drv->trace.by[2] * drv->now_t * drv->now_t + 
				drv->trace.by[3] * drv->now_t * drv->now_t * drv->now_t;

		float next_dx = drv->trace.bx[1] + 2 * drv->trace.bx[2] * drv->now_t + 
			3 * drv->trace.bx[3] * drv->now_t * drv->now_t;

		float next_dy = drv->trace.by[1] + 2 * drv->trace.by[2] * drv->now_t +
			3 * drv->trace.by[3] * drv->now_t * drv->now_t;

		float next_norm = sqrt( next_dx * next_dx + next_dy * next_dy );
		next_norm = 1.0 / next_norm;

		next_dy *= next_norm * ( drv->the_ant->axis_len / 2.0 );
		next_dx *= next_norm * ( drv->the_ant->axis_len / 2.0 );

		float turn_z = now_dx * next_dy - now_dy * next_dx;
		next_x -= now_x;
		next_y -= now_y;

		float left_mov, right_mov;
		left_mov = right_mov = sqrt( next_x * next_x + next_y * next_y );

		next_dx -= now_dx;
		next_dy -= now_dy;
		float turn_mov = sqrt( next_dx * next_dx + next_dy * next_dy );
		
		if( turn_z > 0.0 )
		{
			left_mov -= turn_mov;
			right_mov += turn_mov;
		}
		else if( turn_z < 0.0 )
		{
			left_mov += turn_mov;
			right_mov -= turn_mov;
		}

		left_mov /= drv->the_ant->tire_radius;
		right_mov /= drv->the_ant->tire_radius;

		//drv->the_ant->left_speed += left_mov / TIMEQUANT;
		//drv->the_ant->right_speed += right_mov / TIMEQUANT;
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

