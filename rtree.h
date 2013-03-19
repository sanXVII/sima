#ifndef RTREE_H_
#define RTREE_H_



#define RTREE_CHILDS (4) /* child nodes per parent */
typedef struct rtree_n
{
	float max_x;
	float min_x;
	float max_y;
	float min_y;
	void * val;

	struct rtree_n * parent;
	struct rtree_n * child;
	struct rtree_n * sister;
} rtree_n;


#define RTREE_NBLOCK_SZ (1000)
typedef struct rtree_nblk
{
	rtree_n node[ RTREE_NBLOCK_SZ ];
	struct rtree_nblk * next;
} rtree_nblk;


typedef struct rtree
{
	rtree_n * adam; /* first node */

	rtree_nblk first_blk;
	rtree_nblk * cur_blk;
	int cur_cnt;  
} rtree;


rtree * new_rtree( void );
void del_rtree( rtree * rt );


/* Add point to R-tree. Not thread-safe function
 * rt   - main data
 * x, y - point coordinates
 * p    - pointer to customer data */
void to_rtree( rtree * rt, float x, float y, void * val );

/* Print all R-tree to stdout */
void print_rtree( rtree * rt );

#endif /* RTREE_H_ */
