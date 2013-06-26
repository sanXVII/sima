#ifndef WD_FREE_PIX_H
#define WD_FREE_PIX_H


#include "rtree.h"


/* Detected free pixels (chips) */
typedef struct free_pix
{
        float red;
        float green;
        float blue;
        float angle;
        float state; /* 0-free 1-reserved */

	rtree_n * node;
} free_pix;

#define PIX_BLK_SZ 100

typedef struct free_pix_blk
{
        free_pix pix[ PIX_BLK_SZ ];
        struct free_pix_blk * next;

} free_pix_blk;

typedef struct free_pixels
{
	int pix_cnt;
	free_pix_blk * last_blk;
	
	rtree * tree;
} free_pixels;



/* Constructor and destructor of data. */
free_pixels * new_free_pixels( void );
void del_free_pixels( free_pixels * das );


/* Add new detected free pixel.
 *
 *         das - general pixels data structure
 *   x,y,angle - position on boiard
 *       r,g,b - pixel color
 */
free_pix * add_pixel( free_pixels * das, float x, float y,
			float r, float g, float b, float angle );

/* Search for next near free pixel.
 *
 *         das - general pixels data structure
 *        cpix - current search position
 */
free_pix * find_next_pixel( free_pixels * das, free_pix * cpix, 
				float x, float y, float delta );



#endif /*WD_FREE_PIX_H*/

