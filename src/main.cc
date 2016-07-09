#include <QApplication>
#include <QMessageBox>
#include <Application/VikingViewApp.h>
#include <Application/CommandLineArgs.h>
#include <iostream>

#ifdef _WIN32

#include <windows.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <iostream>
#include <fstream>

//#include <Data/AlphaShape.h>

#ifndef _USE_OLD_IOSTREAMS
using namespace std;
#endif

// maximum number of lines the output console should have
static const WORD MAX_CONSOLE_LINES = 500;

void RedirectIOToConsole2()
{
  int hConHandle;
  long lStdHandle;

  CONSOLE_SCREEN_BUFFER_INFO coninfo;
  FILE* fp;

  // allocate a console for this app
  AllocConsole();

  // set the screen buffer to be big enough to let us scroll text
  GetConsoleScreenBufferInfo( GetStdHandle( STD_OUTPUT_HANDLE ), &coninfo );
  //coninfo.dwSize.Y = MAX_CONSOLE_LINES;

  coninfo.dwSize.X = 10000;
  coninfo.dwSize.Y = 10000;
  SetConsoleScreenBufferSize( GetStdHandle( STD_OUTPUT_HANDLE ), coninfo.dwSize );

  SMALL_RECT windowSize = {0, 0, 79, 49};

  // Change the console window size:
  SetConsoleWindowInfo( GetStdHandle( STD_OUTPUT_HANDLE ), TRUE, &windowSize );

  //  SetConsoleWindowInfo()

  // redirect unbuffered STDOUT to the console
  lStdHandle = (long)GetStdHandle( STD_OUTPUT_HANDLE );
  hConHandle = _open_osfhandle( lStdHandle, _O_TEXT );
  fp = _fdopen( hConHandle, "w" );
  *stdout = *fp;
  setvbuf( stdout, NULL, _IONBF, 0 );

  // redirect unbuffered STDIN to the console
  lStdHandle = (long)GetStdHandle( STD_INPUT_HANDLE );
  hConHandle = _open_osfhandle( lStdHandle, _O_TEXT );
  fp = _fdopen( hConHandle, "r" );
  *stdin = *fp;
  setvbuf( stdin, NULL, _IONBF, 0 );

  // redirect unbuffered STDERR to the console
  lStdHandle = (long)GetStdHandle( STD_ERROR_HANDLE );
  hConHandle = _open_osfhandle( lStdHandle, _O_TEXT );
  fp = _fdopen( hConHandle, "w" );
  *stderr = *fp;
  setvbuf( stderr, NULL, _IONBF, 0 );

  // make cout, wcout, cin, wcin, wcerr, cerr, wclog and clog
  // point to console as well
  ios::sync_with_stdio( true );
}

#endif // _WIN32

int main( int argc, char** argv )
{

#ifdef WIN32
  ::SetErrorMode( 0 );
  RedirectIOToConsole2();
#endif
  try {

    //cgal_main(argc, argv);

    std::cerr << "VikingView initializing...\n";

    QApplication app( argc, argv );

	// Collect and parse the command line arguments into a reusable object
	QSharedPointer<CommandLineArgs> command_line_args =
		QSharedPointer<CommandLineArgs>( new CommandLineArgs( argc, argv ));

    QSharedPointer<VikingViewApp> studio_app =
        QSharedPointer<VikingViewApp>( new VikingViewApp( command_line_args ) );

    studio_app->show();

    studio_app->initialize_vtk();

    //studio_app->load_structure(180);

	// Process the id command by loading the cells for every
	// id provided as a parameter
    if ( command_line_args->command_used( "id" ) )
	{
	  QList< QString > id_parameters = command_line_args->command_parameters( "id" );
	  for ( int i = 0; i < id_parameters.size(); ++i)
	  {
        int id = id_parameters[ i ].toInt();
        studio_app->load_structure( id );
      }
	}

	// Process the export command by exporting the render scene
	// in each format specified as a parameter
	if ( command_line_args->command_used( "export" ))
	{
	  // Only run export operations if a single cell was loaded
	  if ( !command_line_args->command_used( "id" ) 
		|| command_line_args->command_parameters( "id" ).size() != 1 )
   	  {
		QMessageBox::critical( 0, "Export error", "Error! Tried to export cell geometry with no or multiple cell ids specified (use exactly 1, i.e. -id 593 )" );
		return;
	  }

	  // If no filename is provided at the command line, use a default
	  QString filename = "VikingViewExport";
	  
	  if ( command_line_args->command_used( "filename" ) )
	  { 
		if ( command_line_args->command_parameters( "filename" ).size() != 1 )
		{ 
		  QMessageBox::critical(0, "Export error", "Error! Tried to set export filename with no or multiple paths specified (give 1 filename, i.e. -filename test )");
		  return;
		}

		filename = command_line_args->command_parameters( "filename" ).back();
	  }

	  QList< QString > export_file_types = command_line_args->command_parameters("export");

	  for ( int i = 0; i < export_file_types.size(); ++i )
	  {
		QString export_type = export_file_types[ i ];

		// TODO put pointers to VikingViewApp's two different export functions in a map, 
		// then call the export function mapped to the file type
	  }
	}
		/*arg == "-export-dae" )
      {
        QString filename = argv[argidx++];
        studio_app->export_dae( filename );
        return 0;
      }
	  else if (arg == "-export-obj")
	  {
		  QString filename = argv[argidx++];
		  studio_app->export_obj( filename );
		  return 0;
	  }
      else
      {
        std::cerr << "unrecognized option: " << arg.toStdString() << "\n";
      }
    }*/

    /*

       // do this after "show" for mac initialization
       studio_app->initialize_vtk();

       if ( argc == 2 )
       {
       studio_app->open_project( QString( argv[1] ) );
       }

     */
    return app.exec();
  }
  catch ( std::exception e )
  {
    std::cerr << "Exception caught!" << std::endl;
    std::cerr << e.what() << "\n";
  }
}
