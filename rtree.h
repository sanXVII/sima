#ifndef RTREE_H_
#define RTREE_H_


typedef struct rtree_n
{
} rtree_n;


#define RTREE_NBLOCK_SZ (1000)
typedef struct rtree_nblk
{
	rtree_n node[ RTREE_NBLOCK_SZ ];
	struct rtree_nblk * next;
} rtree_nblk;


typedef struct rtree
{
  
  
	rtree_nblk first_blk;
	rtree_nblk * cur_blk;
	int cur_cnt;
  
} rtree;


rtree * new_rtree( void );
void del_rtree( rtree * rt );


/* Add point to R-tree
 * rt   - main data
 * x, y - point coordinates
 * p    - pointer to customer data */
void to_rtree( rtree * rt, float x, float y, void * p );


#endif /* RTREE_H_ */
