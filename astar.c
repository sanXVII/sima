

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "astar.h"




static void add_new_block( astar * ad )
{
	astar_nblock * new = ( astar_nblock * )malloc( sizeof( astar_nblock ) );
	assert( new );
	memset( new, 0, sizeof( astar_nblock ) );
	
	ad->nblk_cnt++;

	/* Hook at the end of chain */
	assert( ad->cur_blk );
	ad->cur_blk->next = new;

	/* Reallocate heap array */
	astar_n ** nh = ( astar_n ** )malloc(
		sizeof( astar_n * ) * ad->nblk_cnt * ASTAR_NBLOCK_SZ );
	assert( nh );

	memcpy( nh, ad->opens_heap, sizeof( astar_n * ) * ad->opens_num );
	free( ad->opens_heap );
	ad->opens_heap = nh;
}


static astar_n * get_new_node( astar * ad )
{
	astar_n * ret = ad->cur_blk->node + ad->n_use_num % ASTAR_NBLOCK_SZ;
	ad->n_use_num++;

	if( !( ad->n_use_num % ASTAR_NBLOCK_SZ ) )
	{
		if( !ad->cur_blk->next )
		{
			add_new_block( ad );
		}
		ad->cur_blk = ad->cur_blk->next;
	}

	memset( ret, 0, sizeof( astar_n ) );
	return ret;
}


astar * new_astar( void )
{
	astar * new = ( astar * )malloc( sizeof( astar ) );
	assert( new );
	memset( new, 0, sizeof( astar ) );

	new->cur_blk = &( new->first_blk );
	new->nblk_cnt = 1/* first block */;

	new->opens_heap = ( astar_n ** )malloc( 
		sizeof( astar_n * ) * new->nblk_cnt * ASTAR_NBLOCK_SZ );
	assert( new->opens_heap );

	return new;
}

void delete_astar( astar * ad )
{
	free( ad->opens_heap );

	astar_nblock * cb = ad->first_blk.next;
	while( cb )
	{
		astar_nblock * kill = cb;
		cb = cb->next;

		free( kill );
	}
	free( ad );
}



static void add_to_heap( astar * ad, astar_n * node )
{
}

int make_astar( astar * ad, int len )
{
	/* Clear previus route. */
	ad->n_use_num = 0;
	ad->opens_num = 0;
	ad->cur_blk = &( ad->first_blk );
	
	/* Mark first node. */
	astar_n * s = get_new_node( ad );

	s->g = 0.0;
	s->h = ( float )len;
	s->f = s->g + s->h;

	add_to_heap( ad, s );

	while( ad->opens_num )
	{
	}
	
	return 0;
}

