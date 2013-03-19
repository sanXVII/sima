#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "rtree.h"


rtree * new_rtree( void )
{
	rtree * new = ( rtree * )malloc( sizeof( struct rtree ) );
	assert( new );

	memset( new, 0, sizeof( struct rtree ) );
	new->cur_blk = &( new->first_blk );

	return new;
}

void del_rtree( rtree * rt )
{
	rtree_nblk * ct = rt->first_blk.next;
	while( ct )
	{
		rtree_nblk * kill = ct;
		ct = ct->next;

		free( kill );
	}
	free( rt );
}


static rtree_n * get_new_node( rtree * rt )
{
	rtree_n * rval = rt->cur_blk->node + rt->cur_cnt;
	rt->cur_cnt++;

	if( rt->cur_cnt == RTREE_NBLOCK_SZ )
	{
		rt->cur_cnt = 0;
		rtree_nblk * new = ( rtree_nblk * )malloc( sizeof( rtree_nblk ) );
		assert( new );

		memset( new, 0, sizeof( rtree_nblk ) );
		rt->cur_blk->next = new;
		rt->cur_blk = new;
	}
	return rval;
}


/* Catch child node to parent. */
static void to_rtree_node( rtree * rt, rtree_n * parent, rtree_n * child )
{
	if( parent->val ) /* is leaf */
	{
		rtree_n * nn = get_new_node( rt );
		nn->max_x = nn->min_x = parent->max_x;
		nn->max_y = nn->min_y = parent->max_y;
		nn->val = parent->val;
		nn->parent = parent;

		parent->val = 0l; /* not is leaf */
		parent->child[ RTREE_CHILDS - 1 ] = nn;
	}

	/* Check parets borders. */
	parent->max_x = ( child->max_x > parent->max_x ) ? child->max_x : parent->max_x;
	parent->max_y = ( child->max_y > parent->max_y ) ? child->max_y : parent->max_y;
	parent->min_x = ( child->min_x < parent->min_x ) ? child->min_x : parent->min_x;
	parent->min_y = ( child->min_y < parent->min_y ) ? child->min_y : parent->min_y;

	float min_dS;
	int best_i = 0; /* to first child */

	int i;
	for( i = 0; i < RTREE_CHILDS; i++ )
	{
		rtree_n * tnod = parent->child[ i ];
		if( !tnod )
		{
			parent->child[ i ] = child;
			child->parent = parent;
			return;
		}

		float max_x = ( child->max_x > tnod->max_x ) ? child->max_x : tnod->max_x;
		float max_y = ( child->max_y > tnod->max_y ) ? child->max_y : tnod->max_y;
		float min_x = ( child->min_x < tnod->min_x ) ? child->min_x : tnod->min_x;
		float min_y = ( child->min_y < tnod->min_y ) ? child->min_y : tnod->min_y;

		float dS = ( max_x - min_x ) * ( max_y - min_y );
		dS -= ( tnod->max_x - tnod->min_x ) * ( tnod->max_y - tnod->min_y );

		min_dS = !i ? dS : min_dS; /* init value */
		if( min_dS > dS ) /* look for minimal change required child */
		{
			min_dS = dS;
			best_i = i;
		}
	}

	to_rtree_node( rt, parent->child[ best_i ], child );
}

void to_rtree( rtree * rt, float x, float y, void * val )
{
	rtree_n * nn = get_new_node( rt );
	nn->max_x = nn->min_x = x;
	nn->max_y = nn->min_y = y;
	nn->val = val;

	if( !rt->adam )
	{
		rt->adam = nn;
	}
	else
	{
		to_rtree_node( rt, rt->adam, nn );
	}
}


static void print_node( rtree_n * n )
{
	if( n->val ) /* is_leaf */
	{
		/* Show full path for this leaf */
		rtree_n * show = n;
		while( show )
		{
			printf( "(x: %0.2f..%0.2f y: %0.2f..%0.2f)[%p] -> ", show->min_x, 
				show->max_x, show->min_y, show->max_y, show->val );
			show = show->parent;
		}
		printf( "\n" );
		return;
	}

	int i;
	for( i = 0; i < RTREE_CHILDS; i++ )
	{
		if( n->child[ i ] )
		{
			print_node( n->child[ i ] );
		}
	}
}

void print_rtree( rtree * rt )
{
	print_node( rt->adam );
}

