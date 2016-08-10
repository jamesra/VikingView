#ifndef VIKINGVIEW_APPLICATION_COMMANDLINEARGS_H
#define VIKINGVIEW_APPLICATION_COMMANDLINEARGS_H

#include <QString>
#include <QMap>

class CommandLineArgs
{
  public:

    CommandLineArgs( int argc, char** argv );

    bool command_used( QString command );
    bool command_has_parameter( QString command, QString parameter );

    QList< QString > command_parameters( QString command );

  private:

    static QList< QString > command_prefixes;

	QString arg_as_command( QString arg );

    QList< QString > commands_;
    QMap< QString, QList<QString> > command_parameters_;
};

#endif
