

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

#include <math.h>


#include "wd_gui.h"
#include "wd_of_ants.h"
#include "sim_drv.h"
#include "ant_furer.h"


/* */
static pthread_t gui_pthread_id;
//static pthread_attr_t gui_pthread_attr;

//-----------------------------------------

/* adjust these accordingly */
static char fontpath[] = "./kelson/kelson_sans_light.ttf";
static int screenwidth = 800;
static int screenheight = 700;

static int done = 0; /* GUI is running while zero. */

static float z_4eye = -6.0f; /* Camera position (z) */
static float vz_4eye = 0.0f; /* */


static wd_of_ants * world;




static int nextpoweroftwo(int x)
{
	double logbase2 = log(x) / log(2);
	return ( int )( pow(2,ceil(logbase2)) + 0.5 );
}

static char *init_sdl(SDL_Surface** screen)
{
	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER))
		return SDL_GetError();
	
	*screen = SDL_SetVideoMode(screenwidth, screenheight, 0, SDL_OPENGL);
	
	SDL_WM_SetCaption("C-Junkie's SDLGL text example", 0);
	
	if(TTF_Init())
		return TTF_GetError();
	
	return 0;
}

static void SDL_GL_RenderText(char *text, 
                      TTF_Font *font,
                      SDL_Color color,
                      SDL_Rect *location)
{
	SDL_Surface *initial;
	SDL_Surface *intermediary;
	SDL_Rect rect;
	int w,h;
	int texture;
	
	/* Use SDL_TTF to render our text */
	initial = TTF_RenderText_Blended(font, text, color);
	
	/* Convert the rendered text to a known format */
	w = nextpoweroftwo(initial->w);
	h = nextpoweroftwo(initial->h);
	
	intermediary = SDL_CreateRGBSurface(0, w, h, 32, 
			0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);

	SDL_BlitSurface(initial, 0, intermediary, 0);
	
	/* Tell GL about our new texture */
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, w, h, 0, GL_BGRA, 
			GL_UNSIGNED_BYTE, intermediary->pixels );
	
	/* GL_NEAREST looks horrible, if scaled... */
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);	

	/* prepare to render our texture */
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture);
	glColor3f(1.0f, 1.0f, 1.0f);
	
	/* Draw a quad at location */
	glBegin(GL_QUADS);
		/* Recall that the origin is in the lower-left corner
		   That is why the TexCoords specify different corners
		   than the Vertex coors seem to. */
		glTexCoord2f(0.0f, 1.0f); 
			glVertex2f(location->x    , location->y);
		glTexCoord2f(1.0f, 1.0f); 
			glVertex2f(location->x + w, location->y);
		glTexCoord2f(1.0f, 0.0f); 
			glVertex2f(location->x + w, location->y + h);
		glTexCoord2f(0.0f, 0.0f); 
			glVertex2f(location->x    , location->y + h);
	glEnd();
	
	/* Bad things happen if we delete the texture before it finishes */
	glFinish();
	
	/* return the deltas in the unused w,h part of the rect */
	location->w = initial->w;
	location->h = initial->h;
	
	/* Clean up */
	SDL_FreeSurface(initial);
	SDL_FreeSurface(intermediary);
	glDeleteTextures(1, &texture);
}

static void glEnable2D()
{
	int vPort[4];
  
	glGetIntegerv(GL_VIEWPORT, vPort);
  
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
  
	glOrtho(0, vPort[2], 0, vPort[3], -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
}

static void glDisable2D()
{
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();   
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();	
}

static void init_gl()
{
	/* Irrelevant stuff for this demo */
	glShadeModel(GL_SMOOTH);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClearDepth(1.0f);
	glDepthFunc(GL_LEQUAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	
	/* Required if you want alpha-blended textures (for our fonts) */
	glBlendFunc(GL_ONE, GL_ONE);
	glEnable(GL_BLEND);
	
	/* Required setup stuff */
	glViewport(0, 0, screenwidth/*800*/, screenheight /*600*/);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0f, screenwidth / (float)screenheight, 0.1f, 50.0f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();	
}








static void key_down( SDLKey sym )
{
	switch( sym )
	{
        	case SDLK_LEFT:
                	break;
        	case SDLK_RIGHT:
        		break;
        	case SDLK_UP:
			vz_4eye += 0.05f;
        		break;
        	case SDLK_DOWN:
			vz_4eye -= 0.05f;
                        break;
		default:
			break;
	}
}

static void key_up( SDLKey sym )
{
	switch( sym )
	{
        	case SDLK_LEFT:
                	break;
        	case SDLK_RIGHT:
        		break;
        	case SDLK_UP:
			vz_4eye = 0.0f;
        		break;
        	case SDLK_DOWN:
			vz_4eye = 0.0f;
                        break;
		default:
			break;
	}
}


static void * gui_entry( void * args )
{
	SDL_Surface *screen;
	TTF_Font* font;
	char *err;
	SDL_Color color;
	SDL_Rect position;
	SDL_Event event;
	
	/* Do boring initialization */
	if((err = init_sdl(&screen))) {
		printf("Error while initializing: %s", err);
		return 0l; /* must be returned error code */
	}
	
	if(!(font = TTF_OpenFont(fontpath, 19))) {
		printf("Error loading font: %s", TTF_GetError());
		return 0l; /* must be returned error code */
	}
	
	init_gl();

	while( !done ) {
		/* render a fun litte quad */
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glLoadIdentity();

		z_4eye += vz_4eye;
		glTranslatef(0.0f, 0.0f, z_4eye);
		glDisable(GL_TEXTURE_2D);
	
		int ix, iy;
		for( iy = 0; iy < 16; iy++ )
		{
			for( ix = 0; ix < 16; ix++ )
			{
				if( !( ( ix + iy ) % 2 ) ) continue;

				glBegin(GL_QUADS);
				glNormal3f(0.0f, 0.0f, 1.0f);
				glColor3f(0.1f, 0.1f, 0.1f);

				glVertex3f(-0.09f - 1.5 + ix * 0.2, -0.09f - 1.5 + iy * 0.2,  0.0f);
				glVertex3f(-0.09f - 1.5 + ix * 0.2, 0.09f - 1.5 + iy * 0.2,  0.0f);
				glVertex3f( 0.09f - 1.5 + ix * 0.2, 0.09f - 1.5 + iy * 0.2,  0.0f);
				glVertex3f( 0.09f - 1.5 + ix * 0.2, -0.09f - 1.5 + iy * 0.2, 0.0f);
				glEnd();
			}
		}

		//glBegin(GL_LINES);
		//glColor3f(1.0f, 0.5f, 0.5f);

		//glVertex3f(0.0f, 0.0f, 0.0f);
		//glVertex3f(0.1f, 0.0f, 0.0f);

		//glVertex3f(0.0f, 0.0f, 0.0f);
		//glVertex3f(0.0f, 0.1f, 0.0f);

		//glVertex3f(0.0f, 0.0f, 0.0f);
		//glVertex3f(0.0f, 0.0f, 0.1f);
		//glEnd();

		/* Show ants */
		ant * cant = world->muvis;
		while( cant )
		{
			glPushMatrix();
			glTranslatef( cant->pos_x, cant->pos_y, 0.1 );
			glRotatef( cant->pos_ang * 180 / M_PI, 0.0, 0.0, 1.0 );

			glBegin( GL_LINES );
			glColor3f( 1.0f, 1.0f, 1.0f );
			glVertex3f( 0.0, -1 * cant->axis_len / 2, 0.0 );
			glVertex3f( 0.0, +1 * cant->axis_len / 2, 0.0 );
			glVertex3f( 0.1, 0.0, 0.0 );
			glVertex3f( 0.0, 0.0, 0.0 );

			glColor3f( 0.1f, 0.5f, 0.5f );
			glVertex3f( 0.0, cant->corr_turn, 0.0 );
			glVertex3f( 0.0, 0.0, 0.0 );
			glEnd();

			/* Tyres */
			glPushMatrix();
			glTranslatef( 0.0, -1 * cant->axis_len / 2, 0.0 );
			glRotatef( cant->right_angle * 180 / M_PI, 0.0, 1.0, 0.0 );

			glBegin( GL_LINES );
			glColor3f( 1.0f, 0.0f, 0.0f );
			glVertex3f( cant->tire_radius, 0.0, 0.0 );
			glVertex3f( 0.0, 0.0, 0.0 );
			glEnd();

			glBegin( GL_LINE_LOOP );
			glColor3f( 0.7f, 0.7f, 0.7f );
			glVertex3f( cant->tire_radius, 0.0, 0.0 );
			glVertex3f( 0.0, 0.0, cant->tire_radius );
			glVertex3f( -1 * cant->tire_radius, 0.0, 0.0 );
			glVertex3f( 0.0, 0.0, -1 * cant->tire_radius );
			glEnd();

			glPopMatrix();

			glPushMatrix();
			glTranslatef( 0.0, cant->axis_len / 2, 0.0 );
			glRotatef( cant->left_angle * 180 / M_PI, 0.0, 1.0, 0.0 );

			glBegin( GL_LINES );
			glColor3f( 1.0f, 0.0f, 0.0f );
			glVertex3f( cant->tire_radius, 0.0, 0.0 );
			glVertex3f( 0.0, 0.0, 0.0 );
			glEnd();

			glBegin( GL_LINE_LOOP );
			glColor3f( 0.7f, 0.7f, 0.7f );
			glVertex3f( cant->tire_radius, 0.0, 0.0 );
			glVertex3f( 0.0, 0.0, cant->tire_radius );
			glVertex3f( -1 * cant->tire_radius, 0.0, 0.0 );
			glVertex3f( 0.0, 0.0, -1 * cant->tire_radius );
			glEnd();
			glPopMatrix();
			
			glPopMatrix();
			cant = cant->next;
		}

		/* Show simple drivers on board. */
		sim_drv * sdrv = all_simple_drivers();
		while( sdrv )
		{
			if( sdrv->act_task.status )
			{
				glPushMatrix();
				glTranslatef( sdrv->act_task.tg_x, sdrv->act_task.tg_y, 0.0 );
				glRotatef( sdrv->act_task.tg_ang * 180 / M_PI, 0.0, 0.0, 1.0 );

				glBegin( GL_LINES );
				glColor3f( 1.0f, 0.7f, 0.7f );
				glVertex3f( -0.2, 0.0, 0.0 );
				glVertex3f( 0.1, 0.0, 0.0 );
				glVertex3f( 0.0, -0.1, 0.0 );
				glVertex3f( 0.0, 0.1, 0.0 );
				glEnd();

				glPopMatrix();

				glBegin( GL_LINE_STRIP );
				glColor3f( 0.7f, 0.7f, 0.7f );
				float t;
				for( t = 0.0; t < 1.0; t += 0.05 )
				{
					glVertex3f( sdrv->trace.bx[0] + sdrv->trace.bx[1]*t +
						sdrv->trace.bx[2]*t*t + sdrv->trace.bx[3]*t*t*t, 
						sdrv->trace.by[0] + sdrv->trace.by[1]*t +
						sdrv->trace.by[2]*t*t + sdrv->trace.by[3]*t*t*t, 
						0.0 );
				}
				glEnd();

				t = sdrv->now_t;
				float tx = sdrv->trace.bx[0] + sdrv->trace.bx[1]*t +
					sdrv->trace.bx[2]*t*t + sdrv->trace.bx[3]*t*t*t;
				float ty = sdrv->trace.by[0] + sdrv->trace.by[1]*t +
					sdrv->trace.by[2]*t*t + sdrv->trace.by[3]*t*t*t;

				glBegin( GL_LINES );
				glColor3f( 1.0f, 0.7f, 0.7f );
				glVertex3f( tx - 0.05, ty - 0.05, 0.0 );
				glVertex3f( tx + 0.05, ty + 0.05, 0.0 );
				glVertex3f( tx - 0.05, ty + 0.05, 0.0 );
				glVertex3f( tx + 0.05, ty - 0.05, 0.0 );
				glEnd();
			}
			sdrv = sdrv->next;
		}


		
		/* Go in HUD-drawing mode */
		glEnable2D();
		glDisable(GL_DEPTH_TEST);
		
		/* Draw some text */
		color.r = 255;
		color.g = 255;
		color.b = 255;
		/** A quick note about position.
		 * Enable2D puts the origin in the lower-left corner of the
		 * screen, which is different from the normal coordinate
		 * space of most 2D api's. position, therefore,
		 * gives the X,Y coordinates of the lower-left corner of the
		 * rectangle **/
		position.x = 40;
		position.y = 60;

		static int tcnt = 0; tcnt++;
		char tstr[100];
		sprintf( tstr, "Number of steps: %lu", world->sim_cnt );

		SDL_GL_RenderText( tstr, font, color, &position );
		position.y -= position.h;
		SDL_GL_RenderText("A line right underneath", font, color, &position);
		position.y -= position.h;
		SDL_GL_RenderText("Yay text rendering.", font, color, &position);

		/* Come out of HUD mode */
		glEnable(GL_DEPTH_TEST);
		glDisable2D();
		
		/* Show the screen */
		SDL_GL_SwapBuffers();
		
		while( SDL_PollEvent( &event ))
		{	
			switch (event.type)
			{
			case SDL_QUIT:
				done++;
				break;

			case SDL_KEYDOWN:
				key_down( event.key.keysym.sym );
				break;

			case SDL_KEYUP:
				key_up( event.key.keysym.sym );
				break;
			//case SDL_KEYDOWN:
			//	done++;
			//	break;
			}
		}

		SDL_Delay( 30/* ms */ );
	}

	/* Clean up (the atexit's take care of the rest) */
	TTF_CloseFont(font);

	TTF_Quit();
	SDL_Quit();
	
	return 0l;
}
//-----------------------------------------

int init_gui( void )
{
        //pthread_attr_init( &gui_pthread_attr );
        //pthread_attr_setdetachstate( &gui_pthread_attr, PTHREAD_CREATE_JOINABLE );

	world = get_world();

        return pthread_create( &gui_pthread_id, 0L, gui_entry, 0L ); 
}

void close_gui( void )
{
	/* It called from main thread. */
	done++;

	void * value;
	assert( !pthread_join( gui_pthread_id, &value) );
}


