#include <stdio.h>

#include "rtree.h"

int main( void )
{
	rtree * tree = new_rtree();

	int i;
	for( i = 1; i <= 1000000; i++ )
	{
		to_rtree( tree, (float)( rand() % 1000 ) - 500.0,
				(float)( rand() % 1000 ) - 500.0, ( void * )i );
	}
	print_rtree( tree );

	del_rtree( tree );
	return 0;
}

