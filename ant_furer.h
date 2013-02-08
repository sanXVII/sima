#ifndef MASTER_H_
#define MASTER_H_

/* Subsystem of General actions planning. */

typedef struct task
{
	float tg_x;
	float tg_y;
	float tg_ang;
	int status; /* 0-empty 1-new */
} task;


/* Padding.. */
struct ant;
struct wd_of_ants;


/* Return 0 if OK, else not OK. */
int get_next_task( task * pt, struct ant * pa, struct wd_of_ants * wd );



#endif /* MASTER_H_ */
