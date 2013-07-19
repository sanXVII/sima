#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

#include <SDL.h>
#include <SDL_image.h>

#include "wd_of_ants.h"
#include "wd_gui.h"



int main_done = 0; /* Global "done" event */



static void print_usage( const char * prog )
{
	printf( "Usage: %s [key]\n", prog );
}

static void print_help()
{
	printf( "Simulation for Ants action.\n\n" );
	printf( "Keys:\n" );
	printf( "     -h, --help                Print this help information about mosaic ants simulator.\n" );
	printf( "     -i, --image <file>        Image file.\n" );
	printf( "\n" );
}




static unsigned long getpixel(SDL_Surface *surface, int x, int y)
{
	int bpp = surface->format->BytesPerPixel;
	/* Here p is the address to the pixel we want to retrieve */
	unsigned char *p = (unsigned char *)surface->pixels + y * surface->pitch + x * bpp;

	switch(bpp) 
	{
	case 1:
		return *p;
		break;

	case 2:
		return *(Uint16 *)p;
		break;

	case 3:
		if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
			return p[0] << 16 | p[1] << 8 | p[2];
		else
			return p[0] | p[1] << 8 | p[2] << 16;
		break;

	case 4:
		return *( unsigned long *)p;
		break;

	default:
		return 0;       /* shouldn't happen, but avoids warnings */
	}
}


static void load_image( const char * filename, genplan * plan )
{
	int t_h = plan->width;
	int t_w = plan->hight;

	SDL_Surface * image;

	image = IMG_Load( filename );
	if( !image )
	{
		/* image failed to load */
		printf("IMG_Load: %s\n", IMG_GetError());
		return;
	}

	/* print some info about the image */
	printf("loaded %s: %dx%d %dbpp\n", filename,
		image->w, image->h, image->format->BitsPerPixel);

	float step_x = image->w;
	step_x /= t_w;
	float step_y = image->h;
	step_y /= t_h;

	/* remap */
	// SDL_GetRGB( getpixel( image, int x, int y ), image->format, Uint8 *r, Uint8 *g, Uint8 *b);
	int h, w;
	float r_sum, g_sum, b_sum;
	int cntr;
	int base_x, base_y, x, y;
	for( h = 0; h < t_h; h++ )
	{
		for( w = 0; w < t_w; w++ )
		{
			pix * pixel = plan->pixs + w + h * t_w;

			cntr = 0;
			r_sum = g_sum = b_sum = 0;
			base_x = ( int )( step_x * ( float )w );
			base_y = ( int )( step_y * ( float )h );
			for( x = 0; x < (int)step_x; x++ )
			{
				for( y = 0; y < (int)step_y; y++ )
				{
					unsigned char r, g, b;
					SDL_GetRGB( getpixel( image, base_x + x, base_y + y ), image->format, &r, &g, &b );
					r_sum += r; g_sum += g; b_sum += b;
					cntr++;
				}
			}
			r_sum /= cntr * 256; g_sum /= cntr * 256; b_sum /= cntr * 256;

			pixel->state = 0;
			pixel->red = r_sum;
			pixel->green = g_sum;
			pixel->blue = b_sum;
		}
	}

	SDL_FreeSurface( image );
}



int main( int argc, char ** argv )
{
	/* Loaing keys and options */
	char * arg_image_name = 0;

	int vn;
	int arg_state = 0; /* Key name state */
	for( vn = 1; vn < argc; vn++ )
	{
		switch( arg_state )
		{
		case 0: /* Key name state */
			if( !strcmp( "-h", argv[ vn ] ) || !strcmp( "--help", argv[ vn ] ) )
			{
				print_usage( argv[0] );
				print_help();
				return 0;
			}
			else if( !strcmp( "-i", argv[ vn ] ) || !strcmp( "--image", argv[ vn ] ) )
			{
				arg_state = 1;
			}
			break;
		case 1: /* Image filename */
			if( ( !argv[ vn ][ 0 ] ) || ( argv[ vn ][ 0 ] == '-' ) )
			{
				printf( "Invalid filename.\n" );
				return -1;
			}
			arg_image_name = argv[ vn ];
			printf( "Image file: '%s'.\n", arg_image_name );
			arg_state = 0;
			break;
		default:
			break;
		}
	}


	if( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_TIMER ) )
	{
		printf( "fail: %s\n", SDL_GetError() );
		return -1;
	}
		

	/* Chek all data and run world */
	wd_of_ants_init();
	init_gui();
	if( arg_image_name ) load_image( arg_image_name, &( get_world()->plan ) );


	/* Prepare stdin */
	int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
	fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);

	
	char cmd[ 256/* max cmd len */ ];
	while( 1 )
	{
		/* Enter new command string. */
		int clen = 0;
		while( !clen )
		{
			printf( "cmd: " );
			while( 1 ) 
			{
				int c = getchar();
				if( c == '\n' ) break;
				if( c != EOF )
				{
					cmd[ clen ] = c;
					clen++;
					if( clen >= ( sizeof( cmd ) - 1 ) ) break;
				}
			
				if( main_done ) goto exit;	
				usleep( 1000 ); /* Polling interval */
			}
			cmd[ clen ] = 0; /* End of string */
		}

		if( main_done ) goto exit;
		
		/* Execute entered command. */
		if( !strcmp( cmd, "h" ) )
		{
			printf( "List of supported commands:\n" );
			printf( "     h - Print help information.\n" );
			printf( "    fs - Start fast simulation.\n" );
			printf( "    rs - Start realtime simulation.\n" );
			printf( "    cb - Clear barriers.\n" );
			printf( "   arb - Add random stubs.\n" );
			printf( "   ab1 - Add 2 horizontal walls.\n" );
			printf( "   ab2 - Add vertical walls.\n" );
			printf( "   img <filename> - Load image.\n " );
			printf( "     q - Quit.\n" );
			printf( "\n" );
		}
		else if( !strncmp( cmd, "img ", 4 ) )
		{
			char buf[1024];
			if( sscanf( cmd + 4, "%s", buf ) == 1 )
			{
				printf( "Load file '%s'\n", buf );
				load_image( buf, &( get_world()->plan ) );
			}
			else
			{
				printf( "Invalid filename.\n" );
			}
		}
		else if( !strcmp( cmd, "q" ) )
		{
			main_done++; /* breaking all threads */
			break;
		}
		else if( !strcmp( cmd, "fs" ) )
		{
			while( !main_done ) { wd_of_ants_run(); }
			break;
		}
		else if( !strcmp( cmd, "rs" ) )
		{
			while( !main_done ) { wd_of_ants_run(); usleep( 1000 ); }
			break;
		}
		else if( !strcmp( cmd, "cb" ) )
		{
			reset_barriers();
		}
		else if( !strcmp( cmd, "arb" ) )
		{
			add_random_barriers();
		}
		else if( !strcmp( cmd, "ab1" ) )
		{
			add_barriers_1();
		}
		else if( !strcmp( cmd, "ab2" ) )
		{
			add_barriers_2();
		}
		else
		{
			printf( "Unknown command `%s`. Enter `h` for help.\n\n", cmd );
		}
		//wd_of_ants_run();
	}
exit:	
	/* Here we must to close all other threads ... */
	close_gui();

	wd_of_ants_destroy();
	SDL_Quit();

	return 0;
}
