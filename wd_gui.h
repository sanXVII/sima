#ifndef WD_GUI_H_
#define WD_GUI_H_


/* Return 0 if OK */
int init_gui( void );

/* Must be called before exit from main thread. */
void close_gui( void );


void run_rtime_sim( void ); /* Run realtime simulation */
extern int main_done; /* Global done event from/to main.c */

#endif /* WD_GUI_H_ */
