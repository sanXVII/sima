
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include <stdio.h>


#include "sim_drv.h"
#include "wd_of_ants.h"
#include "astar.h"


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

	new->a_star = new_astar();
}


static void make_sp3_seg( sp3_seg * p, float beg_x, float beg_y, float beg_ang,
					float end_x, float end_y, float end_ang )
{
	/* Polynome Inputs. */
	float beg_dx = cos( beg_ang );
	float beg_dy = sin( beg_ang );

	float end_dx = cos( end_ang );
	float end_dy = sin( end_ang );

	/* Polynome. */
	p->bx[0] = beg_x;
	p->bx[1] = beg_dx;
	p->bx[2] = 3 * ( end_x - beg_x ) - 2 * beg_dx - end_dx;
	p->bx[3] = 2 * ( beg_x - end_x ) + beg_dx + end_dx;	

	p->by[0] = beg_y;
	p->by[1] = beg_dy;
	p->by[2] = 3 * ( end_y - beg_y ) - 2 * beg_dy - end_dy;
	p->by[3] = 2 * ( beg_y - end_y ) + beg_dy + end_dy;	
}


static void reset_task( sim_drv * drv )
{
	get_next_task( &( drv->act_task ), 
		drv->the_ant, drv->world );

	int i;
	float bx = drv->the_ant->pos_x;
	float by = drv->the_ant->pos_y;
	float bang = drv->the_ant->pos_ang;
	float ex;
	float ey;
	float eang;

	for( i = 0; i < MAX_SP3; i++ )
	{
		/* Random point */
		if( i == MAX_SP3 - 1 )
		{
			ex = drv->act_task.tg_x;
			ey = drv->act_task.tg_y;
			eang = drv->act_task.tg_ang;
		}
		else
		{
			ex = ( float )( rand() % 100 ) / 25.0 - 2.0;
			ey = ( float )( rand() % 100 ) / 25.0 - 2.0;
			eang = M_PI * ( float )( rand() % 100 ) / 50.0;
		}

		make_sp3_seg( &( drv->trace[ i ] ), bx, by, bang, ex, ey, eang );

		bx = ex;
		by = ey;
		bang = eang;
	}
	drv->sp3_num = i;
	drv->cur_sp3 = 0;
	drv->now_t = 0.0; /* begin of spline */

	/* A* */
	astar_n * route = make_astar( drv->a_star, drv->the_ant->pos_x, 
			drv->the_ant->pos_y, drv->act_task.tg_x, drv->act_task.tg_y );
	
}


void exec_sim_drv( void )
{
	sim_drv * drv = drivers;

	while( drv )
	{
		float time_plus = 0.002;

		/* Check for task. */
		if( !drv->act_task.status ) 
		{
			reset_task( drv );
		}

		if( drv->now_t + time_plus > 1.0 )
		{
			drv->cur_sp3++;
			drv->now_t = 0.0; /* begin of spline */

			if( drv->cur_sp3 == drv->sp3_num )
			{
				reset_task( drv );
			}
		}

		/* Test route */

		sp3_seg * trace = drv->trace + drv->cur_sp3;

		/* Find compensation of error. */
		float now_x = trace->bx[0] + trace->bx[1] * drv->now_t +
				trace->bx[2] * drv->now_t * drv->now_t + 
				trace->bx[3] * drv->now_t * drv->now_t * drv->now_t;
		float now_y = trace->by[0] + trace->by[1] * drv->now_t +
				trace->by[2] * drv->now_t * drv->now_t + 
				trace->by[3] * drv->now_t * drv->now_t * drv->now_t;

		float ex = now_x - drv->the_ant->pos_x;
		float ey = now_y - drv->the_ant->pos_y;

		float corr_turn = ex * sin( drv->the_ant->pos_ang * (-1) ) +  
				ey * cos( drv->the_ant->pos_ang * (-1) );
		float mov = ex * cos( drv->the_ant->pos_ang * (-1) ) -
				ey * sin( drv->the_ant->pos_ang * (-1) );

		if( corr_turn < 0.0 )
		{
			corr_turn *= (-1) * corr_turn;
		}
		else
		{
			corr_turn *= corr_turn;
		}

		/* Lag correction value. */
		mov = mov < -1.0 ? -1.0 : mov;
		mov += 1.0;
		
		drv->the_ant->left_speed = 0.0;
		drv->the_ant->right_speed = 0.0;

		/* Offset correction turn. */
		drv->the_ant->left_speed -= corr_turn * 500.0;
		drv->the_ant->right_speed += corr_turn * 500.0;

//printf( "Correction control: left_w=%f .. right_w=%f .. mov = %f", drv->the_ant->left_speed, drv->the_ant->right_speed, mov );

		/* Major control. */
		float now_dx = trace->bx[1] + 2 * trace->bx[2] * drv->now_t + 
			3 * trace->bx[3] * drv->now_t * drv->now_t;

		float now_dy = trace->by[1] + 2 * trace->by[2] * drv->now_t +
			3 * trace->by[3] * drv->now_t * drv->now_t;

		float now_norm = sqrt( now_dx * now_dx + now_dy * now_dy );

		float c_angle = asin( now_dy / now_norm );
		c_angle = now_dx < 0.0 ? M_PI - c_angle : c_angle;
		c_angle -= drv->the_ant->pos_ang;
		c_angle = sin( c_angle );

		/* Angle correction turn. */
		drv->the_ant->left_speed -= c_angle;
		drv->the_ant->right_speed += c_angle;
//printf( ".. sin( c_angle ) =%f\n", c_angle );

		now_norm = 1.0 / now_norm;
		
		now_dx *= now_norm * ( drv->the_ant->axis_len / 2.0 );
		now_dy *= now_norm * ( drv->the_ant->axis_len / 2.0 );
		
		drv->now_t += time_plus; /* Move out */

		float next_x = trace->bx[0] + trace->bx[1] * drv->now_t +
				trace->bx[2] * drv->now_t * drv->now_t + 
				trace->bx[3] * drv->now_t * drv->now_t * drv->now_t;
		float next_y = trace->by[0] + trace->by[1] * drv->now_t +
				trace->by[2] * drv->now_t * drv->now_t + 
				trace->by[3] * drv->now_t * drv->now_t * drv->now_t;

		float next_dx = trace->bx[1] + 2 * trace->bx[2] * drv->now_t + 
			3 * trace->bx[3] * drv->now_t * drv->now_t;

		float next_dy = trace->by[1] + 2 * trace->by[2] * drv->now_t +
			3 * trace->by[3] * drv->now_t * drv->now_t;

		float next_norm = sqrt( next_dx * next_dx + next_dy * next_dy );
		next_norm = 1.0 / next_norm;

		next_dy *= next_norm * ( drv->the_ant->axis_len / 2.0 );
		next_dx *= next_norm * ( drv->the_ant->axis_len / 2.0 );

		float turn_z = now_dx * next_dy - now_dy * next_dx;
		next_x -= now_x;
		next_y -= now_y;

		float left_mov, right_mov;
		left_mov = right_mov = sqrt( next_x * next_x + next_y * next_y ) * mov;

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

		drv->the_ant->left_speed += left_mov / TIMEQUANT;
		drv->the_ant->right_speed += right_mov / TIMEQUANT;
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

		delete_astar( t_drv->a_star );
		free( t_drv );
	}
}

