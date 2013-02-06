
#include <stdlib.h>

#include "sim_drv.h"
#include "wd_of_ants.h"


/* Chain of simple drivers. */
static sim_drv * drivers = 0l;


void add_sim_drv( wd_of_ants * world, ant * c_ant )
{
	sim_drv * new = ( sim_drv * )malloc( sizeof( struct sim_drv ) );
	
	new->the_ant = c_ant;
	new->world = world;

	/* Here we must init all data structures. */


	new->next = drivers;
	drivers = new;
}


void exec_sim_drv( void )
{
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

