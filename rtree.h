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


#define RTREE_NBLOCK_SZ (5000)
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

	int not_del_cnt; /* for thread-safe */
} rtree;


rtree * new_rtree( void );
void del_rtree( rtree * rt );


/* For multipoint read access */
void not_del_rtree_pls( rtree * rt );
void thx_may_del_rtree( rtree * rt );


/* Add point to R-tree. Not thread-safe function
 * rt   - main data
 * x, y - point coordinates
 * p    - pointer to customer data */
void to_rtree( rtree * rt, float x, float y, void * val );

/* Find near node.
 *
 * cur_n - current near node. We want to find next.
 *         Put rt->adam to this argument, for first search.
 * ( x +- delta, y +- delta ) - Region of search.
 *
 * Return pointer to the wanted object or NULL, if not found.
 * See rtree_n structure and you may extract void * val to your application data. 
 */
rtree_n * get_next_near( rtree_n * cur_n, float x, float y, float delta );


/* Print all R-tree to stdout */
void print_rtree( rtree * rt );

#endif /* RTREE_H_ */
