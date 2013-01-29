#include "wd_gui.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <pthread.h>
#include <stdarg.h>

#include <SDL.h>
#include <SDL_ttf.h>
#include <GL/gl.h>
#include <GL/glu.h>



/* */
static pthread_t gui_pthread_id;
static pthread_attr_t gui_pthread_attr;



#define WIDTH 800
#define HEIGHT 600
#define BPP 4
#define DEPTH 32

static void WriteText( SDL_Surface * screen, int x, int y, const char * text, int sz, int r, int g, int b)
{
    SDL_Color clr; // Тип для цвета. 4 числа — R, G, B и A, соответственно.
    clr.r = r; 
    clr.g = g;  // Зададим параметры цвета
    clr.b = b;

    // Загружаем шрифт по заданному адресу размером sz
    TTF_Font * fnt = TTF_OpenFont("./kelson/kelson_sans_regular.ttf", sz);
    assert( fnt );

    SDL_Rect dest;
    dest.x = x;
    dest.y = y;

    // Переносим на поверхность текст с заданным шрифтом и цветом
    SDL_Surface * TextSurface = TTF_RenderText_Blended( fnt, text, clr );
    assert( TextSurface );

    SDL_BlitSurface( TextSurface, NULL, screen, &dest );
    SDL_FreeSurface( TextSurface ); // Освобождаем память уже ненужной поверхности
    TTF_CloseFont( fnt ); // Закрываем шрифт
}


//static void setpixel(SDL_Surface *screen, int x, int y, Uint8 r, Uint8 g, Uint8 b)
//{
//    Uint32 *pixmem32;
//    Uint32 colour;  
 
//    colour = SDL_MapRGB( screen->format, r, g, b );
  
//    pixmem32 = (Uint32*) screen->pixels  + y + x;
//    *pixmem32 = colour;
//}


static void DrawScreen(SDL_Surface* screen, int h)
{ 
//    int x, y, ytimesw;
  
    if( SDL_MUSTLOCK( screen )) 
    {
        if( SDL_LockSurface( screen ) < 0 ) return;
    }

//    for(y = 0; y < screen->h; y++ ) 
//    {
//        ytimesw = y*screen->pitch/BPP;
//        for( x = 0; x < screen->w; x++ ) 
//        {
//            setpixel(screen, x, ytimesw, (x*x)/256+3*y+h, (y*y)/256+x+h, h);
//        }
//    }
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glLoadIdentity();
	glBegin( GL_QUADS );
		glColor3f( 1, 0, 0 ); glVertex3f( 0, 0, 0 );
		glColor3f( 1, 1, 0 ); glVertex3f( 100, 0, 0 );
		glColor3f( 1, 0, 1 ); glVertex3f( 100, 100, 0 );
		glColor3f( 1, 1, 1 ); glVertex3f( 0, 100, 0 );
	glEnd();
	SDL_GL_SwapBuffers();


	static int cnt = 0; cnt++;
	char text[ 100 ];
	sprintf( text, "Hello World %i", cnt );
	WriteText( screen, 100/* x */, 100/* y */, text, 15/* sz */, 255/* R */, 255/* G */, 255/* B */);

    if(SDL_MUSTLOCK(screen)) SDL_UnlockSurface(screen);
  
    SDL_Flip(screen); 
}


static void * gui_entry( void * args )
{
    SDL_Surface *screen;
    SDL_Event event;
  
    int keypress = 0;
    int h=0; 
  
    if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
    {
	fprintf( stderr, "error: SDL init failed\n" );
	return 0l;
    }

    if( TTF_Init()==-1 ) 
    {
    	printf("TTF_Init: %s\n", TTF_GetError());
    	return 0l;
    }

   
	if( !( screen = SDL_SetVideoMode( WIDTH, HEIGHT, DEPTH, /*SDL_FULLSCREEN|*/SDL_HWSURFACE | SDL_GL_DOUBLEBUFFER | SDL_OPENGL )))
	{
		SDL_Quit();
		fprintf( stderr, "error: SDL_SetVideoMode failed\n" );
		return 0l;
	}
	glClearColor( 0, 0, 0, 0 );
	glViewport( 0, 0, WIDTH, HEIGHT );
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();

	glOrtho( 0, WIDTH, HEIGHT, 0, 1, -1 );
	glMatrixMode( GL_MODELVIEW );

	glEnable( GL_TEXTURE_2D );
	glLoadIdentity();

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

    TTF_Quit();
    SDL_Quit();
  
    return 0l;
}

int init_gui( void )
{
        pthread_attr_init( &gui_pthread_attr );
        pthread_attr_setdetachstate( &gui_pthread_attr, PTHREAD_CREATE_DETACHED );

        return pthread_create( &gui_pthread_id, 0L, gui_entry, 0L );
}


