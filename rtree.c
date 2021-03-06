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
	new->not_del_cnt++;
	return new;
}

void del_rtree( rtree * rt )
{
	rt->not_del_cnt--;
	if( rt->not_del_cnt )return;

	rtree_nblk * ct = rt->first_blk.next;
	while( ct )
	{
		rtree_nblk * kill = ct;
		ct = ct->next;

		free( kill );
	}
	free( rt );
}

void not_del_rtree_pls( rtree * rt )
{
	rt->not_del_cnt++;
}

void thx_may_del_rtree( rtree * rt )
{
	del_rtree( rt );
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
//static int bcnt = 0;
//printf( "block cnt = %i ..\n", bcnt );
//bcnt++;
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
		parent->child = nn;
	}

	/* Check parets borders. */
	parent->max_x = ( child->max_x > parent->max_x ) ? child->max_x : parent->max_x;
	parent->max_y = ( child->max_y > parent->max_y ) ? child->max_y : parent->max_y;
	parent->min_x = ( child->min_x < parent->min_x ) ? child->min_x : parent->min_x;
	parent->min_y = ( child->min_y < parent->min_y ) ? child->min_y : parent->min_y;

	float min_dS;
	rtree_n * best_child = parent->child; /* to first child */

	int i;
	rtree_n * tnod = parent->child;
	for( i = 0; i < RTREE_CHILDS; i++ )
	{
		if( !tnod )
		{
			child->sister = parent->child;
			parent->child = child;
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
			best_child = tnod;
		}
		tnod = tnod->sister;
	}

	to_rtree_node( rt, best_child, child );
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


static int is_good_xy( rtree_n * n, float x, float y, float delta )
{
	if( x < ( n->min_x - delta ) ) return 0;
	if( x > ( n->max_x + delta ) ) return 0;
	if( y < ( n->min_y - delta ) ) return 0;
	if( y > ( n->max_y + delta ) ) return 0;
	return 1; /* Good node */
}

static rtree_n * get_down( rtree_n * cur_n, float x, float y, float delta )
{
	while( cur_n->child )
	{
		if( !is_good_xy( cur_n, x, y, delta ) ) break;
		cur_n = cur_n->child;
	}
	return cur_n;
}

static rtree_n * get_right( rtree_n * cur_n )
{
	while( cur_n )
	{
		if( cur_n->sister )
		{
			return cur_n->sister;
		}

		cur_n = cur_n->parent;
	}

	return cur_n;
}

rtree_n * get_next_near( rtree_n * cur_n, float x, float y, float delta )
{
	if( !cur_n ) return 0l;

	rtree_n * next = cur_n;
	int to_right = 0;
	while( 1 )
	{
		next = to_right ? get_right( next ) : get_down( next, x, y, delta );
		to_right = ~to_right;
		if( !next ) break;

		if( next->val && ( next != cur_n ) )
		{
			if( is_good_xy( next, x, y, delta ) ) break;
		}
	}
	return next;
}

static void print_node( rtree_n * n )
{
	if( n->val ) /* is_leaf */
	{
		/* Show full path for this leaf */
		rtree_n * show = n;
		while( show )
		{
			printf( "(x:%0.2f..%0.2f y:%0.2f..%0.2f)[%p] -> ", show->min_x, 
				show->max_x, show->min_y, show->max_y, show->val );
			show = show->parent;
		}
		printf( "\n" );
		return;
	}

	rtree_n * cur_n = n->child;;
	while( cur_n )
	{
		print_node( cur_n );
		cur_n = cur_n->sister;
	}
}

void print_rtree( rtree * rt )
{
	print_node( rt->adam );
}

