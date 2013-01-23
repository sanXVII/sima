#include <stdio.h>
#include <stdlib.h>
#include "wd_of_ants.h"




static wd_of_ants my_world;


static void add_muvi_ant( wd_of_ants * wd, float x, float y, float ang )
{
	ant * new = ( ant * )malloc( sizeof( struct ant ) );
	memset( new, 0, sizeof( struct ant ) );	
	
	new->pos_x = x;
	new->pos_y = y;
	new->pos_ang = ang;
	
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
		

		cant = cant->next;
	}
}

