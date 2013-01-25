#include "wd_gui.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <stdarg.h>
#include <SDL.h>



/* */
static pthread_t gui_pthread_id;
static pthread_attr_t gui_pthread_attr;



#define WIDTH 640
#define HEIGHT 480
#define BPP 4
#define DEPTH 32

static void setpixel(SDL_Surface *screen, int x, int y, Uint8 r, Uint8 g, Uint8 b)
{
    Uint32 *pixmem32;
    Uint32 colour;  
 
    colour = SDL_MapRGB( screen->format, r, g, b );
  
    pixmem32 = (Uint32*) screen->pixels  + y + x;
    *pixmem32 = colour;
}


static void DrawScreen(SDL_Surface* screen, int h)
{ 
    int x, y, ytimesw;
  
    if(SDL_MUSTLOCK(screen)) 
    {
        if(SDL_LockSurface(screen) < 0) return;
    }

    for(y = 0; y < screen->h; y++ ) 
    {
        ytimesw = y*screen->pitch/BPP;
        for( x = 0; x < screen->w; x++ ) 
        {
            setpixel(screen, x, ytimesw, (x*x)/256+3*y+h, (y*y)/256+x+h, h);
        }
    }

    if(SDL_MUSTLOCK(screen)) SDL_UnlockSurface(screen);
  
    SDL_Flip(screen); 
}


static void * gui_entry( void * args )
{
    SDL_Surface *screen;
    SDL_Event event;
  
    int keypress = 0;
    int h=0; 
  
    if (SDL_Init(SDL_INIT_VIDEO) < 0 )
    {
	fprintf( stderr, "error: SDL init failed\n" );
	return 0l;
    }
   
    if (!(screen = SDL_SetVideoMode(WIDTH, HEIGHT, DEPTH, /*SDL_FULLSCREEN|*/SDL_HWSURFACE)))
    {
        SDL_Quit();
	fprintf( stderr, "error: SDL_SetVideoMode failed\n" );
        return 0l;
    }
    SDL_WM_SetCaption("Hello world", 0l);
  
    while(!keypress) 
    {
         DrawScreen(screen,h++);
         while(SDL_PollEvent(&event)) 
         {      
              switch (event.type) 
              {
                  case SDL_QUIT:
	              keypress = 1;
	              break;
                  case SDL_KEYDOWN:
                       keypress = 1;
                       break;
              }
         }
    }

    SDL_Quit();
  
    return 0l;
}

int init_gui( void )
{
        pthread_attr_init( &gui_pthread_attr );
        pthread_attr_setdetachstate( &gui_pthread_attr, PTHREAD_CREATE_DETACHED );

        return pthread_create( &gui_pthread_id, 0L, gui_entry, 0L );
}


