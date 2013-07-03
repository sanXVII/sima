
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "wd_free_pix.h"




free_pixels * new_free_pixels( void )
{
	free_pixels * new = ( free_pixels * )malloc( sizeof( free_pixels ) );
	assert( new );
	memset( new, 0, sizeof( free_pixels ) );

	new->tree = new_rtree();
	assert( new->tree );

	return new;
}


void del_free_pixels( free_pixels * das )
{
	del_rtree( das->tree );
	free_pix_blk * cblk = das->last_blk;

	while( cblk )
	{
		free_pix_blk * kill_that = cblk;
		cblk = cblk->next;

		free( kill_that );
	}

	free( das );
}



void add_pixel( free_pixels * das, float x, float y,
                        float r, float g, float b, float angle )
{
	int b_in = das->pix_cnt % PIX_BLK_SZ;

	if( !( b_in ) )
	{
		free_pix_blk * new = ( free_pix_blk * )malloc( sizeof( free_pix_blk ) );
		assert( new );
		memset( new, 0, sizeof( free_pix_blk ) );

		new->next = das->last_blk;
		das->last_blk = new;
	}

	free_pix * pix = das->last_blk->pix + b_in;
	pix->red = r;
	pix->green = g;
	pix->blue = b;
	pix->angle = angle;
	pix->x = x;
	pix->y = y;
	to_rtree( das->tree, x, y, pix );

	das->pix_cnt++;
}


free_pix * find_next_pixel( free_pixels * das, rtree_n ** search,
                                float x, float y, float delta )
{
	rtree_n * cn = 0l;

	search = !search ? &cn : search;
	*search = !( *search ) ? das->tree->adam : ( *search );
//printf( "Search %p .. %f..%f..%f", *search, x, y, delta  );	
	*search = get_next_near( *search, x, y, delta );
//printf( " ..ret %p\n", *search );

	free_pix * rv = ( *search ) ? ( free_pix * )( *search )->val : 0l;
	return rv;
}


free_pix * booking_free_pix( free_pixels * das, float x, float y,
                        float delta, float r, float g, float b )
{
	free_pix * select = 0l;
	float color_distance = 999.99;

	rtree_n * search = 0l;
	free_pix * pix;
	while( ( pix = find_next_pixel( das, &search, x, y, delta ) ) )
	{

		float c_dist = sqrt( ( r - pix->red ) * ( r - pix->red ) + 
			( g - pix->green ) * ( g - pix->green ) + 
			( b - pix->blue ) * ( b - pix->blue ) );
		if( c_dist < color_distance )
		{
			color_distance = c_dist;
			select = pix;
		}
	}

	return select;
}


