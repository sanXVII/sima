#ifndef WD_GUI_H_
#define WD_GUI_H_


/* Return 0 if OK */
int init_gui( void );

/* Must be called before exit from main thread. */
void close_gui( void );



#endif /* WD_GUI_H_ */
