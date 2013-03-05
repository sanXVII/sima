

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>

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

static astar_n * cut_heap_head( astar * ad )
{
	return 0l; /* 0l if empty */
}

static void check_node( astar * ad, astar_n * parent, astar_n * child, float cost )
{
	/* Check for borders from parent to child ---------- */

	float newg = cost + parent->g;
	if( child->state && ( child->g <= newg ) )
	{
		return;
	}

	child->parent = parent;
	child->g = newg;
	child->h = sqrt( ( child->x - parent->x ) * ( child->x - parent->x ) +
			( child->y - parent->y ) * ( child->y - parent->y ) );
	child->f = child->g + child->h;
	add_to_heap( ad, child );
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

	astar_n * n;
	while( ( n = cut_heap_head( ad ) ) )
	{
		if( ( n->x == 0 ) && ( n->y == len ) )
		{
			/* Path found */
			return 0;
		}

		/* near left */
		n->near_l = n->near_l ? n->near_l : get_new_node( ad );
		n->near_l->near_r = n;
		n->near_l->x = n->x - 1;
		n->near_l->y = n->y;
		check_node( ad, n, n->near_l, ASTAR_SQUARE );

		/* near right */
		n->near_r = n->near_r ? n->near_r : get_new_node( ad );
		n->near_r->near_l = n;
		n->near_r->x = n->x + 1;
		n->near_r->y = n->y;
		check_node( ad, n, n->near_r, ASTAR_SQUARE );

		/* near forward */
		n->near_f = n->near_f ? n->near_f : get_new_node( ad );
		n->near_f->near_b = n;
		n->near_f->x = n->x;
		n->near_f->y = n->y + 1;
		check_node( ad, n, n->near_f, ASTAR_SQUARE );

		/* near back */
		n->near_b = n->near_b ? n->near_b : get_new_node( ad );
		n->near_b->near_f = n;
		n->near_b->x = n->x;
		n->near_b->y = n->y - 1;
		check_node( ad, n, n->near_b, ASTAR_SQUARE );

		astar_n * dn;

		/* near left-forward */
		dn = n->near_l->near_f;
		dn = dn ? dn : get_new_node( ad );
		n->near_l->near_f = dn;
		n->near_f->near_l = dn;
		dn->x = n->x - 1;
		dn->y = n->y + 1;
		check_node( ad, n, dn, ASTAR_DIAGONAL );
		
		/* near right-forward */
		dn = n->near_r->near_f;
		dn = dn ? dn : get_new_node( ad );
		n->near_r->near_f = dn;
		n->near_f->near_r = dn;
		dn->x = n->x + 1;
		dn->y = n->y + 1;
		check_node( ad, n, dn, ASTAR_DIAGONAL );

		/* near right-back */
		dn = n->near_r->near_b;
		dn = dn ? dn : get_new_node( ad );
		n->near_r->near_b = dn;
		n->near_b->near_r = dn;
		dn->x = n->x + 1;
		dn->y = n->y - 1;
		check_node( ad, n, dn, ASTAR_DIAGONAL );

		/* near left-back */
		dn = n->near_l->near_b;
		dn = dn ? dn : get_new_node( ad );
		n->near_l->near_b = dn;
		n->near_b->near_l = dn;
		dn->x = n->x - 1;
		dn->y = n->y - 1;
		check_node( ad, n, dn, ASTAR_DIAGONAL );
	}
	
	return -1; /* A* fail */
}

