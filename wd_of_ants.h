#ifndef WD_OF_ANTS_H_
#define WD_OF_ANTS_H_


/* One ant */
typedef struct ant
{
	float pos_x, pos_y; /* Position (metr) */
	float pos_ang; /* Angle (radian) */
	
	float left_angle; /* Current left tire angle (rad) */
	float right_angle; /* Current right tire angle (rad) */
	
	float left_speed; /* Angle speed of legt tire (rad/sec) */
	float right_speed; /* Right tire (rad/sec) */
	
	float axis_len; /* Width of this ant. (metr) */
	float tire_radius; /* (metr) */
} ant;


/* Main struct for fully describe of one system. */
typedef struct wd_of_ants
{
	int value;

} wd_of_ants;




int wd_of_ants_init( void );





#endif /* WD_OF_ANTS_H_ */
