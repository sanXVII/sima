#ifndef ASTAR_H_
#define ASTAR_H_


/* Algorithm A* */
typedef struct astar_n
{
	float real_x;
	float real_y;

        int x;
        int y;
        int state; /* 0-new, 1-open, 2-close */
	int heap_id;

        float g;
        float h;
        float f;

        struct astar_n * near_f;
        struct astar_n * near_l;
        struct astar_n * near_r;
        struct astar_n * near_b;

        struct astar_n * parent;
        struct astar_n * dao; /* Founded path */
} astar_n;


#define ASTAR_NBLOCK_SZ (1000)
typedef struct astar_nblock
{
	astar_n node[ ASTAR_NBLOCK_SZ ];
	struct astar_nblock * next;

} astar_nblock;

typedef struct astar
{
	int n_use_num; /* Num of used nodes */

	astar_n ** opens_heap;
	int opens_num;

	/* Scalable list of nodes */
	astar_nblock * cur_blk;   /* Last used block */
	astar_nblock first_blk; /* Blocks chain */
	
	int nblk_cnt; /* Count of node blocks */

	/* Translation A* points to barrier points */
	/* X = ax_tx * astarX + ay_tx * astarY + ax_0 */
	/* Y = ax_ty * astarX + ay_ty * astarY + ay_0 */
	float ax_tx;
	float ax_ty;
	float ay_tx;
	float ay_ty;
	float ax_0;
	float ay_0;
} astar;



/* Create and destroy A* data structures */
astar * new_astar( void );
void delete_astar( astar * );


/* bx,by - start point
 * ex,ey - end point
 *
 * Return pointer to first path node.
 *   NULL - fail
 */
astar_n * make_astar( astar * ad, float bx, float by, float ex, float ey );

#define ASTEP (0.1) /* metr */


#endif /* ASTAR_H_ */
