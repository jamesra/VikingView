// qt
#include <QMessageBox>

#include <Application/CommandLineArgs.h>


//-----------------------------------------------------------------------------
QList< QString > CommandLineArgs::command_prefixes;

//-----------------------------------------------------------------------------
CommandLineArgs::CommandLineArgs( int argc, char** argv )
{
  // Process every command line argument as a QString, excluding the first
  // argument, "VikingView.exe"
  for ( int i = 1; i < argc; ++i)
  {
    QString arg( argv[i] );

    // If the argument is a command, store it in the command list
    if ( arg_is_command( arg ))
    {
      commands_.push_back( arg );
      // Give it an empty parameter list
      command_parameters_[ arg ] = QList<QString>();
    }
    // Otherwise the argument must be a parameter to the last command
    // passed
    else
    {
      // Make sure a command has been passed to accept the parameter
      if ( commands_.empty() )
      {
        QMessageBox::critical( 0, "Command line args",
            "A non-command argument was passed before any command was passed to accept it as a parameter." );
        return;
      }

      // Add the parameter to the list for its command
      QString last_command = commands_.back();
      command_parameters_[ last_command ].push_back( arg );
    }
  }
}

//-----------------------------------------------------------------------------
bool CommandLineArgs::command_used( QString command )
{
  return commands_.contains( command );
}

//-----------------------------------------------------------------------------
bool CommandLineArgs::command_has_parameter( QString command, QString parameter )
{
  return command_parameters_[ command ].contains( parameter );
}

//-----------------------------------------------------------------------------
QList< QString > CommandLineArgs::command_parameters( QString command )
{
  return command_parameters_[ command ];
}

//-----------------------------------------------------------------------------
bool CommandLineArgs::arg_is_command( QString arg )
{
  // Construct the list of acceptable prefixes for a command, if necessary
  if ( command_prefixes.empty() )
  {
    // Commands look like any of these examples:
    //  -export
    //  --export
    //  /export
    command_prefixes.push_back( QString( "-" ));
    command_prefixes.push_back( QString( "--" ));
    command_prefixes.push_back( QString( "/" ));
  }

  // Commands can start with any of the different acceptable prefixes
  for ( int i = 0; i < command_prefixes.size(); ++i )
  {
    if ( arg.startsWith( command_prefixes[ i ] ) ) return true;
  }

  // If an arg doesn't have one of these prefixes, it is a parameter, not
  // a command
  return false;
}
