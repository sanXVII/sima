
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include <stdio.h>


#include "sim_drv.h"
#include "wd_of_ants.h"
#include "astar.h"
#include "rtree.h"


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

static int track_is_fail( sp3_seg * sp3, rtree * stubs )
{
	float t;
	for( t = 0.0; t < 1.0; t += 0.005 )
	{
		float x = sp3->bx[0] + sp3->bx[1]*t + sp3->bx[2]*t*t + sp3->bx[3]*t*t*t;
		float y = sp3->by[0] + sp3->by[1]*t + sp3->by[2]*t*t + sp3->by[3]*t*t*t;

		if( get_next_near( stubs->adam, x, y, 0.15/* 15sm */ ) ) return 1/* fail */;
	}

	return 0/* good */;
}

static void make_next_track( sim_drv * drv )
{
	while( 1 )
	{
		if( !drv->route ) return;

		astar_n * pp = drv->route;
		drv->route = drv->route->dao;
		
		float bx = drv->the_ant->pos_x;
		float by = drv->the_ant->pos_y;
		float bang = drv->the_ant->pos_ang;

		float ex = ( drv->route ) ? drv->route->real_x : drv->act_task.tg_x;
		float ey = ( drv->route ) ? drv->route->real_y : drv->act_task.tg_y;

		/* Finish angle */
		float dx = ex - pp->real_x;
		float dy = ey - pp->real_y;
		float norm = sqrt( dx * dx + dy * dy );
		float c_angle = asin( dy / norm );
		c_angle = dx < 0.0 ? M_PI - c_angle : c_angle;

		float eang = ( drv->route ) ? c_angle : drv->act_task.tg_ang;

		sp3_seg sp3;
		make_sp3_seg( &sp3, bx, by, bang, ex, ey, eang );

		if( !track_is_fail( &sp3, drv->world->stub ) )
		{
			drv->sp3 = sp3;
			drv->now_t = 0.0;
		}
		else
		{
			return;
		}
	}
}


static void reset_task( sim_drv * drv )
{
	get_next_task( &( drv->act_task ), 
		drv->the_ant, drv->world );

	/* A* */
	drv->route = make_astar( drv->a_star, drv->world->stub, drv->the_ant->pos_x, 
			drv->the_ant->pos_y, drv->act_task.tg_x, drv->act_task.tg_y );

}


void exec_sim_drv( void )
{
	sim_drv * drv = drivers;

	while( drv )
	{
		float time_plus = 0.002;

		if( !drv->act_task.status )
		{
			reset_task( drv );
			make_next_track( drv );
		}

		/* Check for task. */
		if( drv->now_t + time_plus > 1.0 )
		{
			if( !drv->route )
			{
				reset_task( drv );
				make_next_track( drv );
			}
			else
			{
				make_next_track( drv );
			}
		}

		/* Test route */
		sp3_seg * trace = &( drv->sp3 );

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

