#ifndef SIM_DRV_H_
#define SIM_DRV_H_


/*
 * Simple driver of ant.
 *
 */

#include "ant_furer.h"


/* Segment of cubic spline.*/
typedef struct sp3_seg
{
	float bx[4];
	float by[4];

	float len;	
} sp3_seg;

#define MAX_SP3 10

typedef struct sim_drv
{
	struct ant * the_ant;
	struct wd_of_ants * world;
	struct task act_task;

	int sp3_num; /* Number of segmant */
	struct sp3_seg trace[ MAX_SP3 ];
	
	int cur_sp3; /* Current segment */
	float now_t; /* Position on segment [0.0 .. 1.0). */

	struct astar * a_star;

	struct sim_drv * next;
} sim_drv;


/* Get full chain of simple drivers. */
sim_drv * all_simple_drivers( void );

/* Add simple driver for one ant. */
void add_sim_drv( struct wd_of_ants * world, struct ant * c_ant );

/* Execute driving. */
void exec_sim_drv( void );

/* Closing and freeing all data structures before program exit. */
void close_sim_drv( void );

#endif /* SIM_DRV_H_ */
