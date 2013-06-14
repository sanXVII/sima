#include <stdio.h>
#include <sys/time.h>

#include "rtree.h"



typedef struct l_pnt_t
{
	float x;
	float y;
	void * val;
} l_pnt_t;

#define POINTS_CNT 5000000
static l_pnt_t l_pnt[ POINTS_CNT ];


#define TEST_DATA_CNT (1000)

#define delta (4.0)


int main( void )
{
	rtree * tree = new_rtree();

	printf( "Building Rtree ...\n" );

	struct timeval start_time;
	struct timeval stop_time;

	gettimeofday( &( start_time ), 0l );

	long i;
	for( i = 1; i <= POINTS_CNT; i++ )
	{
		l_pnt[ i ].x = (float)( rand() % 10000 ) - 500.0;
		l_pnt[ i ].y = (float)( rand() % 10000 ) - 500.0;
		l_pnt[ i ].val = ( void * )i;

		to_rtree( tree, l_pnt[ i ].x, l_pnt[ i ].y, l_pnt[ i ].val );
	}
	gettimeofday( &( stop_time ), 0l );
	unsigned long rtree_build_time = stop_time.tv_sec - start_time.tv_sec;
	rtree_build_time *= 1000000;
	rtree_build_time += stop_time.tv_usec;
	rtree_build_time -= start_time.tv_usec;

	//print_rtree( tree );
	rtree_n * cur;



	l_pnt_t test_pnts[ TEST_DATA_CNT ];
	for( i = 0; i < TEST_DATA_CNT; i++ )
	{
		test_pnts[i].x = (float)( rand() % 10000 ) - 5000.0;
		test_pnts[i].y = (float)( rand() % 10000 ) - 5000.0;
	}


	printf( "RTree search test .....\n" );
	int rtree_found_cnt = 0;
	gettimeofday( &( start_time ), 0l );

	for( i = 0; i < TEST_DATA_CNT; i++ )
	{
		float xx = test_pnts[i].x;
		float yy = test_pnts[i].y;

//		printf( "Rtree search %3.2f:%3.2f +-%3.2f\n", xx, yy, delta );
	
		for( cur = get_next_near( tree->adam, xx, yy, delta ); 
			cur != 0; cur = get_next_near( cur, xx, yy, delta ) )
		{
//			printf( "++ point[ %f, %f ] .. val[ %i ]\n", 
//				cur->min_x, cur->min_y, (int)cur->val );
			rtree_found_cnt++;
		}
	}
	gettimeofday( &( stop_time ), 0l );
	unsigned long rtree_search_time = stop_time.tv_sec - start_time.tv_sec;
	rtree_search_time *= 1000000;
	rtree_search_time += stop_time.tv_usec;
	rtree_search_time -= start_time.tv_usec;

	printf( "Linear search test .....\n" );
	int linear_found_cnt = 0;

	gettimeofday( &( start_time ), 0l );
	for( i = 0; i < TEST_DATA_CNT; i++ )
	{
		float xx = test_pnts[i].x;
		float yy = test_pnts[i].y;

		int ii;	
//		printf( "Linear search for %3.2f:%3.2f +-%f\n", xx, yy, delta );
		for( ii = 0; ii < POINTS_CNT; ii++ )
		{
			if( xx < l_pnt[ ii ].x - delta ) continue;
			if( xx > l_pnt[ ii ].x + delta ) continue;
			if( yy < l_pnt[ ii ].y - delta ) continue;
			if( yy > l_pnt[ ii ].y + delta ) continue;
//			printf( "-- point[ %f, %f ] .. val[ %i ]\n",
//				l_pnt[ ii ].x, l_pnt[ ii ].y, (int)l_pnt[ ii ].val );
			linear_found_cnt++;
		}
	}
	gettimeofday( &( stop_time ), 0l );
	unsigned long linear_search_time = stop_time.tv_sec - start_time.tv_sec;
	linear_search_time *= 1000000;
	linear_search_time += stop_time.tv_usec;
	linear_search_time -= start_time.tv_usec;

	printf( "Summary:\n" );
	printf( "Points count: %i\n", POINTS_CNT );
	printf( "RTree data build time: %lu usec\n", rtree_build_time );
	printf( "Search time for %i regions (RTree)/(Linear): (%lu usec) / (%lu usec)\n", 
		TEST_DATA_CNT, rtree_search_time, linear_search_time );
	printf( "Founded poinst (RTree)/(Linear): %i/%i\n", rtree_found_cnt, linear_found_cnt );

	del_rtree( tree );
	return 0;
}

