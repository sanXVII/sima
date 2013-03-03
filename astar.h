#ifndef ASTAR_H_
#define ASTAR_H_


/* Algorithm A* */
typedef struct astar_n
{
        int x;
        int y;
        int state;
	int heap_id;

        float g;
        float h;
        float f;

        struct astar_n * near_f;
        struct astar_n * near_l;
        struct astar_n * near_r;
        struct astar_n * near_b;

        struct astar_n * parent;
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
} astar;

#define ASTAR_SQUARE (0.1)    /* Metr */
#define ASTAR_DIAGONAL (0.14) /* Metr */



/* Create and destroy A* data structures */
astar * new_astar( void );
void delete_astar( astar * );


#endif /* ASTAR_H_ */
