#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "wd_of_ants.h"
#include "wd_gui.h"



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

	/* Chek all data and run world */
	wd_of_ants_init();
	init_gui();
	
	char cmd[ 100 ];
	while( 1 )
	{
		printf( "cmd: " );
		while( scanf( "%s", cmd ) != 1 );

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
		else if( !strcmp( cmd, "img" ) )
		{
			char buf[1024];
			while( scanf( "%s", buf ) != 1);
			printf( "Load file '%s'\n", buf );
		}
		else if( !strcmp( cmd, "q" ) )
		{
			break;
		}
		else if( !strcmp( cmd, "fs" ) )
		{
			while( 1 ) wd_of_ants_run();
		}
		else if( !strcmp( cmd, "rs" ) )
		{
			while( 1 ) { wd_of_ants_run(); usleep( 1000 ); }
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

	/* Here we must to close all other threads ... */
	close_gui();

	wd_of_ants_destroy();

	return 0;
}
