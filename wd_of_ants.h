#ifndef WD_OF_ANTS_H_
#define WD_OF_ANTS_H_

#include "ant_furer.h"
#include "wd_free_pix.h"



/* Planning pixels */
typedef struct pix
{
	float red; /* 0.0 .. 1.0 */
	float green;
	float blue;
	int state; /* 0-empty */
} pix;

typedef struct genplan
{
	float left_up_x;
	float left_up_y;
	float pix_side;

	int width; /* in pixels */
	int hight; /* in pixels */
	pix * pixs;
	int last_tasked; /* index in pixs */

} genplan;

struct rtree;



/* One ant */
typedef struct ant
{
	pix cpix;

	float pos_x, pos_y; /* Position (metr) */
	float pos_ang; /* Angle (radian) */
	
	float left_angle; /* Current left tire angle (rad) */
	float right_angle; /* Current right tire angle (rad) */
	
	float left_speed; /* Angle speed of left tire (rad/sec) */
	float right_speed; /* Right tire (rad/sec) */
	
	float axis_len; /* Width of this ant. (metr) */
	float tire_radius; /* (metr) */

	struct ant * next;
} ant;

/* Main struct for fully describe of one system. */
typedef struct wd_of_ants
{
	struct rtree * stub; /* Barrier points */
	ant * muvis; /* List of muvi ants. */

	struct genplan plan;
	free_pixels * free_pixs;
	free_pixels * free_pixs_4del;

	unsigned long sim_cnt;
} wd_of_ants;


/* Return pointer to main wd_of_ants structure. */
wd_of_ants * get_world( void );

/* Prepare of all data structures. */
int wd_of_ants_init( void );

/* Destroy data structures. */
void wd_of_ants_destroy( void );


/* One step to future. */
void wd_of_ants_run( void );

/* Catching free pixel on field
 *
 */
void ant_catch_pix( wd_of_ants * wd, ant * at, float r, float g, float b );

/* For barriers configuration */
void reset_barriers( void );
void add_random_barriers( void );
void add_barriers_1( void );
void add_barriers_2( void );


#define TIMEQUANT 0.01 /* (Sec) Quant of simulation time. */


#endif /* WD_OF_ANTS_H_ */
