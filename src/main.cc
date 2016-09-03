#include <QApplication>
#include <QMessageBox>
#include <QCommandLineParser>
#include <QRegularExpression>
#include <Application/VikingViewApp.h>
#include <Application/CommandLineArgs.h>
#include <Application/ModelController.h>
#include <iostream>

#ifdef _WIN32

#include <windows.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <iostream>
#include <fstream>

#include <Application/Preferences.h>


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

  std::cout << "Console window allocating" << flush;

  CONSOLE_SCREEN_BUFFER_INFO coninfo;
  FILE* fp;

  // allocate a console for this app
  AllocConsole();
  FILE *pFileCon = NULL;  

  // set the screen buffer to be big enough to let us scroll text
  GetConsoleScreenBufferInfo( GetStdHandle( STD_OUTPUT_HANDLE ), &coninfo );
  //coninfo.dwSize.Y = MAX_CONSOLE_LINES;

  coninfo.dwSize.X = 80;
  coninfo.dwSize.Y = MAX_CONSOLE_LINES;
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

  std::cout << "Console window allocated" << flush;
}

#endif // _WIN32

QSharedPointer<QCommandLineParser> BuildParser()
{
	QSharedPointer<QCommandLineParser> parser = QSharedPointer<QCommandLineParser>(new QCommandLineParser());
	parser->addHelpOption();
	parser->addVersionOption();
	parser->addPositionalArgument("IDs", "Structure IDs to load");

	QCommandLineOption optExport(QStringList() << "e" << "export", "Export meshes to files.  Supports .obj or .dae (COLLADA) extensions.", "ExportPath");
	parser->addOption(optExport);

	QCommandLineOption optStructureColors(QStringList() << "sc" << "structure_colors", "Optional .txt file containing structure colors", "StructureColorsFilename", "StructureColors.txt");
	parser->addOption(optStructureColors);

	QCommandLineOption optStructureTypeColors(QStringList() << "stc" << "type_colors", "Optional .txt file containing structure colors", "StructureTypeColorsFilename", "StructureTypeColors.txt");
	parser->addOption(optStructureTypeColors);

	QCommandLineOption optEndpoint(QStringList() << "u" << "url", "Required, endpoint of Viking OData server.", "endpoint");
	parser->addOption(optEndpoint);

	return parser;
}

int LaunchConsoleVersion(int argc, char** argv)
{
	QCoreApplication app(argc, argv);
	QSharedPointer<QCommandLineParser> cmdArgs = BuildParser();
	cmdArgs->process(app);
	QString StructureColorsFilename = cmdArgs->value("sc");
	QString StructureTypeColorsFilename = cmdArgs->value("stc");
	ColorMapper cmap = ColorMapper(StructureColorsFilename,
								   StructureTypeColorsFilename);

	QStringList id_strings = cmdArgs->positionalArguments(); //Fetches the ID numbers
	QList<long> IDs; 
	for (int i = 0; i < id_strings.count(); i++)
	{
		long ID = id_strings[i].toLongLong();
		IDs.append(ID);
	}
	
	ConsoleProgressReporter pf(0,1.0);
	QString endpoint = cmdArgs->value("u");
	QString export_path = cmdArgs->value("e");

	Preferences::Instance().add_connectome("Default", endpoint);

	ExportStructures(IDs, endpoint, export_path, cmap, pf);
	
	return 0;
}


int LaunchWindowedVersion(int argc, char** argv)
{

#ifdef WIN32
	::SetErrorMode(0);
	RedirectIOToConsole2();
#endif

	try {
		
		QApplication app(argc, argv);

		// Collect and parse the command line arguments into a reusable object
		QSharedPointer<CommandLineArgs> command_line_args =
			QSharedPointer<CommandLineArgs>(new CommandLineArgs(argc, argv));

		QSharedPointer<VikingViewApp> studio_app =
			QSharedPointer<VikingViewApp>(new VikingViewApp(command_line_args));

		studio_app->show();

		studio_app->initialize_vtk();

		//studio_app->load_structure(180);

		// Process the id command by loading the cells for every
		// id provided as a parameter
		if (command_line_args->command_used("id"))
		{
			QList< QString > id_parameters = command_line_args->command_parameters("id");
			QList<long> ids;
			for (int i = 0; i < id_parameters.size(); ++i)
			{
				long id = id_parameters[i].toInt();
				ids.append(id);
			}

			studio_app->load_structures(ids);
		}

		// Process the export command by exporting the render scene
		// in each format specified as a parameter
		if (command_line_args->command_used("export"))
		{
			// Only run export operations if a single cell was loaded
			if (!command_line_args->command_used("id")
				|| command_line_args->command_parameters("id").size() != 1)
			{
				QMessageBox::critical(0, "Export error", "Error! Tried to export cell geometry with no or multiple cell ids specified (use exactly 1, i.e. -id 593 )");
				return 0;
			}

			// If no filename is provided at the command line, use a default
			QString filename = "VikingViewExport";

			if (command_line_args->command_used("filename"))
			{
				QList< QString > filenames = command_line_args->command_parameters("filename");
				if (filenames.size() != 1)
				{
					QMessageBox::critical(0, "Export error", "Error! Tried to set export filename with no or multiple paths specified (give 1 filename, i.e. -filename test )");
					return 0;
				}

				filename = filenames.back();
			}

			QList< QString > export_file_types = command_line_args->command_parameters("export");

			if (export_file_types.empty())
			{
				QMessageBox::critical(0, "Export error", "Error! Tried to export cell without specifying export type (give at least 1 type, i.e. -export obj )");
				return 0;
			}

			for (int i = 0; i < export_file_types.size(); ++i)
			{
				QString export_type = export_file_types[i];
				studio_app->export_cell(filename, export_type);
			}

			return 0;
		}

		return app.exec();
	}
	catch (std::exception e)
	{
		std::cerr << "Exception caught!" << std::endl;
		std::cerr << e.what() << "\n";
	}
}


int main( int argc, char** argv )
{

  try {

    //cgal_main(argc, argv);
	   
    std::cerr << "VikingView initializing...\n";

	QStringList args = QStringList();
	for (int i = 0; i < argc; i++)
	{
		args.append(argv[i]);
	}
	
	QSharedPointer<QCommandLineParser> parser = BuildParser();
	parser->process(args);

	QStringList options = parser->optionNames();  
	if (parser->isSet("e"))
	{
		return LaunchConsoleVersion(argc, argv);
	}
	else
	{
		return LaunchWindowedVersion(argc, argv);
	}
  }
  catch (std::exception e)
  {
	  std::cerr << "Exception caught!" << std::endl;
	  std::cerr << e.what() << "\n";
  }
}

