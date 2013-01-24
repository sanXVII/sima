#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "wd_of_ants.h"




static wd_of_ants my_world;


static void add_muvi_ant( wd_of_ants * wd, float x, float y, float ang )
{
	ant * new = ( ant * )malloc( sizeof( struct ant ) );
	memset( new, 0, sizeof( struct ant ) );	
	
	new->pos_x = x;
	new->pos_y = y;
	new->pos_ang = ang;
	new->tire_radius = 0.1; /* metr */
	new->axis_len = 0.3; /* metr */

	/* Control axis */
	new->left_speed = 1.0;
	new->right_speed = 1.2;
	
	new->next = wd->muvis;
	wd->muvis = new;
}


int wd_of_ants_init( void )
{
	/* My first ant named "Muvi" */
	add_muvi_ant( &my_world, 0, 0, 0 );

	printf( "I am happy say you that: Make the World of Ants complete!\n" );
	return 0;
}

void wd_of_ants_destroy( void )
{
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
printf( "x[%p]=%f .. y[%p]=%f\n", cant, cant->pos_x, cant, cant->pos_y );

		cant = cant->next;
	}
}

