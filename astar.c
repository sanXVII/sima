

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

	/***********************/
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

	return ret;
}


astar * new_astar( void )
{
	astar * new = ( astar * )malloc( sizeof( astar ) );
	assert( new );
	memset( new, 0, sizeof( astar ) );

	new->cur_blk = &( new->first_blk );
	new->nblk_cnt = 1/* first block */;

	return new;
}

void delete_astar( astar * ad )
{
	astar_nblock * cb = ad->first_blk.next;
	while( cb )
	{
		astar_nblock * kill = cb;
		cb = cb->next;

		free( kill );
	}
	free( ad );
}

