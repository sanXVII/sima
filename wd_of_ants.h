#ifndef WD_OF_ANTS_H_
#define WD_OF_ANTS_H_


/* One ant */
typedef struct ant
{
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
	ant * muvis; /* List of muvi ants. */

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



#define TIMEQUANT 0.01 /* (Sec) Quant of simulation time. */


#endif /* WD_OF_ANTS_H_ */
