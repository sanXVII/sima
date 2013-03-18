#include <assert.h>
#include <string.h>

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
	rtree * ct = rt->first_blk.next;
	while( ct )
	{
		rtree * kill = ct;
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
static void to_rtree_node( rtree_n * parent, rtree_n * child )
{
	int i;
	for( i = 0; i < RTREE_CHILDS; i++ )
	{
		/* Если опал в прямоугольник, то рекурсивный вызов и выход */

		/* Если NULL то цепляем и выход */

		/* Смотрим насколько надо растягивать .. если меньше то запомним */
	}

	/* Растянем прямоугольник и рекурсивный вызов и выход */
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
		to_rtree_node( rt->adam, nn );
	}
}

