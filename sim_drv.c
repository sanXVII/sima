
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include <stdio.h>


#include "sim_drv.h"
#include "wd_of_ants.h"
#include "astar.h"
#include "rtree.h"

#define TIME_STEP (0.002)

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

		if( get_next_near( stubs->adam, x, y, 0.13/* 13sm */ ) ) return 1/* fail */;
	}

	return 0/* good */;
}

static int make_next_track( sim_drv * drv, float dst_x, float dst_y, float dst_ang )
{
	int have_track = 0;
	while( 1 )
	{
		astar_n * pp = drv->route;
		if( !pp ) return have_track;
		
		float bx = drv->the_ant->pos_x;
		float by = drv->the_ant->pos_y;
		float bang = drv->the_ant->pos_ang;

		float ex = ( pp->dao ) ? pp->dao->real_x : dst_x;
		float ey = ( pp->dao ) ? pp->dao->real_y : dst_y;

		/* Finish angle */
		float dx = ex - pp->real_x;
		float dy = ey - pp->real_y;
		float norm = sqrt( dx * dx + dy * dy );
		float c_angle = asin( dy / norm );
		c_angle = dx < 0.0 ? M_PI - c_angle : c_angle;

		float eang = ( pp->dao ) ? c_angle : dst_ang;

		sp3_seg sp3;
		make_sp3_seg( &sp3, bx, by, bang, ex, ey, eang );

		if( !track_is_fail( &sp3, drv->world->stub ) )
		{
			drv->sp3 = sp3;
			drv->now_t = 0.0;
			drv->route = pp->dao;
			have_track++;
		}
		else
		{
			return have_track;
		}
	}
}



static moving_ctrl( sim_drv * drv )
{
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
	
	drv->now_t += TIME_STEP; /* Move out */

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
}


void exec_sim_drv( void )
{
	sim_drv * drv = drivers;

	while( drv )
	{
		switch( drv->state )
		{
			case -1: /* reset task for current point */
				drv->the_ant->left_speed = 0.0;
				drv->the_ant->right_speed = 0.0;

				if( reset_task( &( drv->act_task ), drv->the_ant, drv->world ) )
				{
					/* Reset failed. Choose new point on image */
					drv->state = 0;
					goto cont;
				}
				drv->state = 1;
				goto cont;

			case 0: /* waiting for task */
				drv->the_ant->left_speed = 0.0;
				drv->the_ant->right_speed = 0.0;

				if( !get_next_task( &( drv->act_task ), drv->the_ant, drv->world ) )
				{
					drv->state = 1;
				}
				goto cont;

			case 1: /* waiting for track */
				/* A* */
				drv->route = make_astar( drv->a_star, drv->world->stub, drv->the_ant->pos_x,
							drv->the_ant->pos_y, drv->act_task.tg_x, drv->act_task.tg_y );
				if( !drv->route )
				{
					drv->state = -1;
					goto cont;
				}

				if( !make_next_track( drv, drv->act_task.tg_x, drv->act_task.tg_y, drv->act_task.tg_ang ) )
				{
					drv->state = -1;
					goto cont;
				}
				drv->state = 2;
				goto cont;

			case 2: /* go to free chip */
				moving_ctrl( drv );
				if( drv->now_t + TIME_STEP > 1.0 )
				{
					if( !drv->route )
					{
						/* Stop */
						drv->the_ant->left_speed = 0.0;
						drv->the_ant->right_speed = 0.0;
						drv->state = 3;
						drv->pause = 0;

						/* And start chip loading */
						ant_catch_pix( drv->world, drv->the_ant, drv->act_task.tg_r, 
								drv->act_task.tg_g, drv->act_task.tg_b );
					}
					else
					{
						if( !make_next_track( drv, drv->act_task.tg_x, drv->act_task.tg_y, 
										drv->act_task.tg_ang ) )
						{
							drv->state = -1;
							goto cont;
						}
					}
				}
				goto cont;

			case 3: /* Loading */
				drv->pause++;
				if( drv->pause >= 200 )
				{
					if( !drv->the_ant->cpix.state )
					{
						/* Not loaded */
						if( reset_task( &( drv->act_task ), drv->the_ant, drv->world ) )
						{
							drv->state = 0;
							goto cont;
						}
						drv->state = 1; /* New route */
					}
					else
					{
						drv->state = 4;
					}
				}
				goto cont;

			case 4: /* waiting track */
				/* A* */
				drv->route = make_astar( drv->a_star, drv->world->stub, drv->the_ant->pos_x,
							drv->the_ant->pos_y, drv->act_task.dst_x, drv->act_task.dst_y );
				if( !drv->route )
				{
					/* Drop chip & Go To next pixel on genplan */
					drv->the_ant->cpix.state = 0;
					drv->state = 0;
					goto cont;
				}
				if( !make_next_track( drv, drv->act_task.dst_x, drv->act_task.dst_y, drv->act_task.dst_ang ) )
				{
					/* Drop chip & Go To next pixel on genplan */
					drv->the_ant->cpix.state = 0;
					drv->state = 0;
					goto cont;
				}
				drv->state = 5;
				goto cont;

			case 5: /* go to mosaic */
				moving_ctrl( drv );
				if( drv->now_t + TIME_STEP > 1.0 )
				{
					if( !drv->route )
					{
						/* Stop */
						drv->the_ant->left_speed = 0.0;
						drv->the_ant->right_speed = 0.0;
						drv->state = 6;
						drv->pause = 0;

						/* And mounting chip */
						drv->act_task.dst_pix->state++;
						drv->act_task.dst_pix->red = drv->the_ant->cpix.red;
						drv->act_task.dst_pix->green = drv->the_ant->cpix.green;
						drv->act_task.dst_pix->blue = drv->the_ant->cpix.blue;
						drv->the_ant->cpix.state = 0; /* unloading */
					}
					else
					{
						if( !make_next_track( drv, drv->act_task.dst_x, 
							drv->act_task.dst_y, drv->act_task.dst_ang ) )
						{
							/* Drop chip & Go To next pixel on genplan */
							drv->the_ant->cpix.state = 0;
							drv->state = 0;
							goto cont;
						}
					}
				}
				goto cont;

			case 6: /* mounting chip */
				drv->pause++;
				if( drv->pause >= 200 )
				{
					drv->state = 0;
				}
				goto cont;

		}
cont:
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

