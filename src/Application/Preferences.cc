
// qt
#include <QThread>

#include <Application/Preferences.h>


//-----------------------------------------------------------------------------
Preferences& Preferences::Instance()
{
  static Preferences instance;
  return instance;
}

//-----------------------------------------------------------------------------
Preferences::Preferences()
  : settings( "Scientific Computing and Imaging Institute", "VikingView" )
{}

//-----------------------------------------------------------------------------
void Preferences::show_window()
{
  this->preferences_window.set_values_from_preferences();
  this->preferences_window.show();
}

//-----------------------------------------------------------------------------
void Preferences::close_window()
{
  this->preferences_window.close();
}

//-----------------------------------------------------------------------------
QSize Preferences::get_main_window_size()
{
  return this->settings.value( "MainWindow/Size", QSize( 1280, 720 ) ).toSize();
}

//-----------------------------------------------------------------------------
void Preferences::set_main_window_size( QSize size )
{
  this->settings.setValue( "MainWindow/Size", size );
}



//-----------------------------------------------------------------------------
QStringList Preferences::get_connectome_list()
{
  QStringList default_list;
  default_list << "http://connectomes.utah.edu/Rabbit/OData/ConnectomeData.svc";
  QList<QVariant> list = this->settings.value( "Connectomes", default_list).toList();

  QStringList string_list;
  foreach (QVariant var, list)
  {
    string_list << var.toString();
  }
  return string_list;
}

//-----------------------------------------------------------------------------
void Preferences::set_connectome_list(QStringList list)
{
  this->settings.setValue("Connectomes", list);
}


//-----------------------------------------------------------------------------
int Preferences::get_last_connectome()
{
  return this->settings.value("LastConnectome", 0).toInt();
}

//-----------------------------------------------------------------------------
void Preferences::set_last_connectome(int id)
{
  this->settings.setValue("LastConnectome", id);
}


//-----------------------------------------------------------------------------
void Preferences::restore_defaults()
{
  QStringList default_list;
  default_list << "http://connectomes.utah.edu/Rabbit/OData/ConnectomeData.svc";

  this->set_connectome_list(default_list);
  this->set_last_connectome(0);

}
