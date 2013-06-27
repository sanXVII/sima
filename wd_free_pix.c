
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

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



free_pix * add_pixel( free_pixels * das, float x, float y,
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
	pix->node = to_rtree( das->tree, x, y, pix );
printf( "++ Pix %p .. node %p\n", pix, pix->node );

	das->pix_cnt++;
	return pix;
}


free_pix * find_next_pixel( free_pixels * das, free_pix * cpix,
                                float x, float y, float delta )
{
	rtree_n * cn = cpix ? cpix->node : das->tree->adam;
	cn = get_next_near( cn, x, y, delta );

printf( "%p ..\n", cn );
	free_pix * rv = cn ? ( free_pix * )cn->val : 0l;
	return rv;
}



