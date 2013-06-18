#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <stdio.h>

#include "astar.h"
#include "rtree.h"




static void add_new_block( astar * ad )
{
	astar_nblock * new = ( astar_nblock * )malloc( sizeof( astar_nblock ) );
	assert( new );
	memset( new, 0, sizeof( astar_nblock ) );
//printf( "add_new_block %p .. %i bytes\n", new, sizeof( astar_nblock ) );
	
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
//printf( "get_new_node .. n_use_num=%i\n", ad->n_use_num );

	if( !( ad->n_use_num % ASTAR_NBLOCK_SZ ) )
	{
//printf( "!( ad->n_use_num %% ASTAR_NBLOCK_SZ ) .. cur_blk->next=%p\n", ad->cur_blk->next );
		if( !ad->cur_blk->next )
		{
			add_new_block( ad );
		}
		ad->cur_blk = ad->cur_blk->next;
	}

	memset( ret, 0, sizeof( astar_n ) );
//printf( "++++ New node %p ..\n", ret );
	return ret;
}


astar * new_astar( void )
{
	astar * new = ( astar * )malloc( sizeof( astar ) );
	assert( new );
	memset( new, 0, sizeof( astar ) );
//printf( "Add A* %p .. %i bytes\n", new, sizeof( astar ) );

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


static void to_up( astar * ad, astar_n * tail )
{
//printf( ". to_up %p\n", tail );
	while( 1 )
	{
//printf( "zzzzz\n" );
		int tid = tail->heap_id;
		if( !tid /* Tail on heaps head */ ) return;

		int hid = ( tid - 1 ) >> 1;
		astar_n * head = ad->opens_heap[ hid ];

		if( head->f < tail->f ) return;

//printf( "%p <-> %p .. %i <-> %i .. %f <> %f\n", head, tail, hid, tid, head->f, tail->f );	
		tail->heap_id = hid;
		head->heap_id = tid;
		
		ad->opens_heap[ hid ] = tail;
		ad->opens_heap[ tid ] = head;
	}
//int ii = 0;
//for( ii = 0; ii < ad->opens_num; ii++ )
//{
//	printf( "%p -> ", ad->opens_heap[ii] );
//}
//printf( "..\n" );
	
}

static void add_to_heap( astar * ad, astar_n * tail )
{
	tail->state = 1/* open */;
	ad->opens_heap[ ad->opens_num ] = tail;
	tail->heap_id = ad->opens_num;
	ad->opens_num++;
//printf( "- add_to_heap %i .. %p\n", ad->opens_num, tail );

	to_up( ad, tail );
}

static astar_n * cut_heap_head( astar * ad )
{
	if( !ad->opens_num ) return 0l; /* 0l if empty */
	
	astar_n * rval = ad->opens_heap[ 0 ];
	rval->state = 2/* close */;
	
	ad->opens_num--;
	ad->opens_heap[ 0 ] = ad->opens_heap[ ad->opens_num ];
	ad->opens_heap[ 0 ]->heap_id = 0;
	
	int hid = 0;
	astar_n * head = ad->opens_heap[ 0 ];
	while( 1 )
	{
		int tid = 2 * hid + 1;
		
		if( tid >= ad->opens_num ) break;
		if( ( tid + 1 ) < ad->opens_num )
		{
			tid = ( ad->opens_heap[ tid ]->f < 
				ad->opens_heap[ tid + 1 ]->f ) ? tid : ( tid + 1 );
		}
		astar_n * tail = ad->opens_heap[ tid ];
		if( head->f < tail->f ) break;
		
//printf( "%i<>%i .. ", hid, tid );
		head->heap_id = tid;
		tail->heap_id = hid;
		
		ad->opens_heap[ hid ] = tail;
		ad->opens_heap[ tid ] = head;
		
		hid = tid;
	}
//printf( "\nCut head heap %p .. opens_num = %i\n", rval, ad->opens_num );
//int ii = 0;
//for( ii = 0; ii < ad->opens_num; ii++ )
//{
//	printf( "%p -> ", ad->opens_heap[ii] );
//}
//printf( "..\n" );
	
	return rval;
}

static void check_node( astar * ad, rtree * stubs, 
			astar_n * parent, astar_n * child, 
			float cost, int len )
{
	/* Prepare real coordinates */
	child->real_x = ad->ax_tx * ( float )child->x + ad->ay_tx * ( float )child->y + ad->ax_0;
	child->real_y = ad->ax_ty * ( float )child->x + ad->ay_ty * ( float )child->y + ad->ay_0;

	float newg = cost + parent->g;
//printf( "+ Check node %p[%f : %f] newg=%f .. opens=%i\n", child, child->real_x, child->real_y, newg, ad->opens_num );

	/* Check for borders from parent to child ---------- */
	rtree_n * nr = get_next_near( stubs->adam, child->real_x, child->real_y, 0.2/* 20sm */ );
	if( nr )
	{
//printf( "- Opa barrier! ..\n" );
		//newg += 50.0/* m */;
		return;
	}

	if( child->state && ( child->g <= newg ) )
	{
//printf( "- Opa child->g <= newg ..\n" );
		return;
	}

//printf( "- state=%i\n", child->state );

	child->parent = parent;
	child->g = newg;
	child->h = sqrt( ( child->x - 0 ) * ( child->x - 0 ) +
			( child->y - len ) * ( child->y - len ) );
	child->f = child->g + child->h;


	if( child->state == 1/*open*/ )
	{
		to_up( ad, child );
	}
	else
	{
		add_to_heap( ad, child );
	}
//printf( "--\n" );
}

astar_n * make_astar( astar * ad, rtree * stubs, float bx, float by, float ex, float ey )
{
	assert( ad );
	assert( stubs );
//printf( "Reset A* ... blocks cnt=%i\n", ad->nblk_cnt );
	/* Clear previus route. */
	ad->n_use_num = 0;
	ad->opens_num = 0;
	ad->cur_blk = &( ad->first_blk );

	/* A* points tranformation matrix */
	float dist = sqrt( ( ex - bx ) * ( ex - bx ) + ( ey - by ) * ( ey - by ) );
	int len = ( int )( dist / ASTEP );
	if( !len ) return 0l;
	ad->ay_tx = ( ex - bx ) * ASTEP / dist;
	ad->ay_ty = ( ey - by ) * ASTEP / dist;
	ad->ax_tx = ad->ay_ty;
	ad->ax_ty = ( -1.0 ) * ad->ay_tx;
	ad->ax_0 = bx;
	ad->ay_0 = by;
	
	/* Mark first node. */
	astar_n * s = get_new_node( ad );

	s->g = 0.0;
	s->h = ( float )len;
	s->f = s->g + s->h;
	s->x = 0; s->real_x = bx;
	s->y = 0; s->real_y = by;

	add_to_heap( ad, s );

	astar_n * n;
	while( ( n = cut_heap_head( ad ) ) )
	{
//printf( "Check %p .. opens %i\n", n, ad->opens_num );
		if( ( n->x == 0 ) && ( n->y == len ) )
		{
//printf( "Path founded! ..\n" );
			/* Path found */
			while( n->parent )
			{
				n->parent->dao = n;
				n = n->parent;
			}
			return n;
		}

		/* near left */
		n->near_l = n->near_l ? n->near_l : get_new_node( ad );
		n->near_l->near_r = n;
		n->near_l->x = n->x - 1;
		n->near_l->y = n->y;
		check_node( ad, stubs, n, n->near_l, 1.0, len );

		/* near right */
		n->near_r = n->near_r ? n->near_r : get_new_node( ad );
		n->near_r->near_l = n;
		n->near_r->x = n->x + 1;
		n->near_r->y = n->y;
		check_node( ad, stubs, n, n->near_r, 1.0, len );

		/* near forward */
		n->near_f = n->near_f ? n->near_f : get_new_node( ad );
		n->near_f->near_b = n;
		n->near_f->x = n->x;
		n->near_f->y = n->y + 1;
		check_node( ad, stubs, n, n->near_f, 1.0, len );

		/* near back */
		n->near_b = n->near_b ? n->near_b : get_new_node( ad );
		n->near_b->near_f = n;
		n->near_b->x = n->x;
		n->near_b->y = n->y - 1;
		check_node( ad, stubs, n, n->near_b, 1.0, len );

		astar_n * dn;

		/* near left-forward */
		dn = n->near_l->near_f;
//printf( "do dn = %p .. ", dn );
		dn = dn ? dn : get_new_node( ad );
//printf( "posle dn = %p \n", dn );
		n->near_l->near_f = dn; dn->near_b = n->near_l;
		n->near_f->near_l = dn; dn->near_r = n->near_f;
		dn->x = n->x - 1;
		dn->y = n->y + 1;
		check_node( ad, stubs, n, dn, 1.4, len );
		
		/* near right-forward */
		dn = n->near_r->near_f;
		dn = dn ? dn : get_new_node( ad );
		n->near_r->near_f = dn; dn->near_b = n->near_r;
		n->near_f->near_r = dn; dn->near_l = n->near_f;
		dn->x = n->x + 1;
		dn->y = n->y + 1;
		check_node( ad, stubs, n, dn, 1.4, len );

		/* near right-back */
		dn = n->near_r->near_b;
		dn = dn ? dn : get_new_node( ad );
		n->near_r->near_b = dn; dn->near_f = n->near_r;
		n->near_b->near_r = dn; dn->near_l = n->near_b;
		dn->x = n->x + 1;
		dn->y = n->y - 1;
		check_node( ad, stubs, n, dn, 1.4, len );

		/* near left-back */
		dn = n->near_l->near_b;
		dn = dn ? dn : get_new_node( ad );
		n->near_l->near_b = dn; dn->near_f = n->near_l;
		n->near_b->near_l = dn; dn->near_r = n->near_b;
		dn->x = n->x - 1;
		dn->y = n->y - 1;
		check_node( ad, stubs, n, dn, 1.4, len );
//printf( "Check complete %p ..\n", n );
	}
printf( "! Path not found.. n_use_num=%i .. opens_num=%i\n", ad->n_use_num, ad->opens_num );
	
	return 0l; /* A* fail */
}

