#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include "wd_of_ants.h"
#include "sim_drv.h"
#include "rtree.h"
#include "wd_free_pix.h"




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
	new->nose_len = 0.05;

	/* Control axis */
	new->left_speed = 0.0;
	new->right_speed = 0.0;
	
	new->next = wd->muvis;
	wd->muvis = new;

	return new;
}


void reset_barriers( void )
{
	rtree * ob = my_world.stub;
	my_world.stub = new_rtree();
	assert( my_world.stub );

	del_rtree( ob );
}

void add_random_barriers( void )
{
	int i;
	for( i = 0; i < 10; i++ )
	{
		float xx = ( float )( rand() % 100 ) / 25.0 - 2.0;
		float yy = ( float )( rand() % 100 ) / 25.0 - 2.0;
		to_rtree( my_world.stub, xx, yy, ( void * )1 );
	}
}

void add_barriers_1( void )
{
	float xx;
	for( xx = -0.6; xx < 0.6; xx += 0.08 )
	{
		to_rtree( my_world.stub, xx, 0.7, ( void * )1 );
		to_rtree( my_world.stub, xx, -0.7, ( void * )1 );
	}
}

void add_barriers_2( void )
{
	float yy;
	for( yy = -0.6; yy < 0.6; yy += 0.08 )
	{
		to_rtree( my_world.stub, 0.0, yy + 1.0, ( void * )1 );
		to_rtree( my_world.stub, 0.0, yy - 1.0, ( void * )1 );
	}
}


int wd_of_ants_init( void )
{
	memset( &my_world, 0, sizeof( my_world ) );

	/* My first ant named "Muvi. And simple driver. */
	ant * first_ant = add_muvi_ant( &my_world, 0, 0, 0 );
	add_sim_drv( &my_world, first_ant );

	/* Walls and barriers */
	my_world.stub = new_rtree();
	assert( my_world.stub );

	/* Prepare mosaic plan */
	my_world.plan.left_up_x = -2.1/* metr */;
	my_world.plan.left_up_y = 2.1;
	my_world.plan.pix_side = 0.02; /* 2sm */
	my_world.plan.width = 100; /* pixels */
	my_world.plan.hight = 100;

	int pix_num = my_world.plan.width * my_world.plan.hight;
	my_world.plan.pixs = ( pix * )malloc( sizeof( pix ) * pix_num );
	assert( my_world.plan.pixs );
	memset( my_world.plan.pixs, 0, pix_num * sizeof( pix ) );

	int i;
	for( i = 0; i < pix_num; i++ )
	{
		my_world.plan.pixs[ i ].red = ( float )i / ( float )pix_num;
		my_world.plan.pixs[ i ].green = 1.0 - ( float )i / ( float )pix_num;
		my_world.plan.pixs[ i ].blue = 
			my_world.plan.pixs[ i ].red * my_world.plan.pixs[ i ].green;
	}
	

	printf( "I am happy say you that: Make the World of Ants complete!\n" );
	return 0;
}

void wd_of_ants_destroy( void )
{
	if( my_world.free_pixs_4del ) del_free_pixels( my_world.free_pixs_4del );
	if( my_world.free_pixs ) del_free_pixels( my_world.free_pixs );

	free( my_world.plan.pixs );
	del_rtree( my_world.stub );

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

		cant->pos_ang += ( rmove - lmove ) / cant->axis_len;
		cant->pos_ang = (cant->pos_ang > 2*M_PI) ? cant->pos_ang-2*M_PI : cant->pos_ang;
		cant->pos_ang = (cant->pos_ang < -2*M_PI) ? cant->pos_ang+2*M_PI : cant->pos_ang;

//printf( "( rmove - lmove ) / cant->axis_len =%f\n", ( rmove - lmove ) / cant->axis_len );
//printf( "x[%p]=%f .. y[%p]=%f\n", cant, cant->pos_x, cant, cant->pos_y );
//if( ( rand() % 1000 ) > 998 )
//{
//	cant->pos_x += 0.01;
//	cant->pos_y += 0.01;
//	cant->pos_ang += (float)( rand() % 1000 ) / 1000.0;
//}
		cant = cant->next;
	}


	if( !( my_world.sim_cnt % 4000 ) )
	{
		/* Update free pixels locations */
		free_pixels * new_data = new_free_pixels();

		int i;
		for( i = 0; i < 1600; i++ )
		{
			float x = ( float )( rand() % 100 ) / 25.0 - 2.0;
			float y = ( float )( rand() % 100 ) / 25.0 - 2.0;
			float r = ( float )( rand() % 100 ) / 100.0;
			float g = ( float )( rand() % 100 ) / 100.0;
			float b = ( float )( rand() % 100 ) / 100.0;
			float ang = ( float )( rand() % 628 ) / 100.0;

			if( ( x < 0.1 ) && ( y > -0.1 ) ) continue;

			/* We should check for near barriers */
			if( get_next_near( my_world.stub->adam, x, y, 0.45/* 45sm */ ) )
				continue;

			add_pixel( new_data, x, y, r, g, b, ang );
		}

		/* Swappping data and destroy oldies. */
		if( my_world.free_pixs_4del ) del_free_pixels( my_world.free_pixs_4del );
		my_world.free_pixs_4del = my_world.free_pixs;
		my_world.free_pixs = new_data;
	}
	my_world.sim_cnt++;
}


void ant_catch_pix( wd_of_ants * wd, ant * at, float r, float g, float b )
{
	at->cpix.state = 0; /* empty */

	float nx = at->pos_x + cos( at->pos_ang ) * at->nose_len;
	float ny = at->pos_y + sin( at->pos_ang ) * at->nose_len;

	free_pix * bpix = 0l;
	bpix = booking_free_pix( wd->free_pixs, nx, ny,
		0.01/* m */, r, g, b, 13/* only free */ );
	if( !bpix ) return;

printf( "Catch pix .. error=%f .. ant_ang=%f .. pix_ang=%f\n",
sqrt( ( nx - bpix->x )*( nx - bpix->x ) + ( ny - bpix->y )*( ny - bpix->y ) ),
at->pos_ang * 360 / ( 2 * M_PI ), bpix->angle * 360 / ( 2 * M_PI ) );
	/* +++ Here we must add comparison of angles. */
	at->cpix.red = bpix->red;
	at->cpix.green = bpix->green;
	at->cpix.blue = bpix->blue;
	at->cpix.state++;

	bpix->state++;
}


