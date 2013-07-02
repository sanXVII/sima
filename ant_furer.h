#ifndef MASTER_H_
#define MASTER_H_



/* Subsystem of General actions planning. */

/* Padding.. */
struct ant;
struct wd_of_ants;
struct pix;


typedef struct task
{
	float tg_x;
	float tg_y;
	float tg_ang;

	float dst_x;
	float dst_y;
	float dst_ang;

	struct pix * dst_pix;	
} task;

/* Return 0 if OK, else not OK. 
 *
 *     not_completed - if not completed current task
 */
int get_next_task( task * pt, struct ant * pa, struct wd_of_ants * wd, int not_completed );



#endif /* MASTER_H_ */
