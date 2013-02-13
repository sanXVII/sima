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
	float begin_x;
	float begin_dx;
	float end_x;
	float end_dx;
	float bx[4];

	float begin_y;
	float begin_dy;
	float end_y;
	float end_dy;
	float by[4];	

} sp3_seg;

typedef struct sim_drv
{
	struct ant * the_ant;
	struct wd_of_ants * world;
	struct task act_task;

	struct sp3_seg trace;

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
