#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include "wd_of_ants.h"
#include "sim_drv.h"




static wd_of_ants my_world;

wd_of_ants * get_world( void )
{
	return &my_world;
}


static ant * add_muvi_ant( wd_of_ants * wd, float x, float y, float ang )
{
	ant * new = ( ant * )malloc( sizeof( struct ant ) );
	assert( new );

	memset( new, 0, sizeof( struct ant ) );	
	
	new->pos_x = x;
	new->pos_y = y;
	new->pos_ang = ang;
	new->tire_radius = 0.1; /* metr */
	new->axis_len = 0.2; /* metr */

	/* Control axis */
	new->left_speed = 3.0;
	new->right_speed = 5.2;
	
	new->next = wd->muvis;
	wd->muvis = new;

	return new;
}


int wd_of_ants_init( void )
{
	memset( &my_world, 0, sizeof( my_world ) );

	/* My first ant named "Muvi. And simple driver. */
	ant * first_ant = add_muvi_ant( &my_world, 0, 0, 0 );
	add_sim_drv( &my_world, first_ant );

	printf( "I am happy say you that: Make the World of Ants complete!\n" );
	return 0;
}

void wd_of_ants_destroy( void )
{
	close_sim_drv();

	/* Deletion of muvi ants */
	ant * cant = my_world.muvis;
	while( cant )
	{
		ant * rip = cant;
		cant = cant->next;
		free( rip );
	}
}

void wd_of_ants_run( void )
{
	/* Driving of ants. */
	exec_sim_drv();

	/* Time forward. Calculate new positions for ants. */
	ant * cant = my_world.muvis;
	while( cant )
	{
		float lrotate = cant->left_speed * TIMEQUANT;
		float rrotate = cant->right_speed * TIMEQUANT;
//printf( "lrotate = %f .. rrotate = %f\n", lrotate, rrotate );
		cant->left_angle += lrotate;
		cant->right_angle += rrotate;
//printf( "left_angle = %f .. right_angle = %f\n", cant->left_angle, cant->right_angle );

		float lmove = lrotate * cant->tire_radius;
		float rmove = rrotate * cant->tire_radius;
//printf( "lmove = %f .. rmove = %f\n", lmove, rmove );

		float move = ( lmove + rmove ) / 2.0;
//printf( "move = %f\n", move );
		cant->pos_x += cos( cant->pos_ang ) * move;
		cant->pos_y += sin( cant->pos_ang ) * move;

		cant->pos_ang += asin( ( rmove - lmove ) / cant->axis_len );
//printf( "x[%p]=%f .. y[%p]=%f\n", cant, cant->pos_x, cant, cant->pos_y );

		cant = cant->next;
	}

	my_world.sim_cnt++;
}

