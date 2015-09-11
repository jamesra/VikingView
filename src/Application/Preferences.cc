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
{
/*
   this->default_connectomes_ << "http://connectomes.utah.edu/Rabbit/OData/ConnectomeData.svc";
   this->default_connectomes_ << "http://websvc1.connectomes.utah.edu/RC1/OData/ConnectomeData.svc";
   this->default_connectomes_ << "http://websvc1.connectomes.utah.edu/RC2/OData/ConnectomeData.svc";
   this->default_connectomes_ << "http://websvc1.connectomes.utah.edu/RPC1/OData/ConnectomeData.svc";
 */
  this->default_connectomes_ << "http://websvc1.connectomes.utah.edu/RC1/OData";
  this->default_connectomes_ << "http://websvc1.connectomes.utah.edu/RC2/OData";
  this->default_connectomes_ << "http://websvc1.connectomes.utah.edu/RPC1/OData";

/*
   this->default_connectome_nicknames_ << "Rabbit";
   this->default_connectome_nicknames_ << "RC1";
   this->default_connectome_nicknames_ << "RC2";
   this->default_connectome_nicknames_ << "RPC1";
 */
  this->default_connectome_nicknames_ << "RC1 (ODATA4)";
  this->default_connectome_nicknames_ << "RC2 (ODATA4)";
  this->default_connectome_nicknames_ << "RPC1 (ODATA4)";

  // ODATA 4 update
  bool odata4 = this->settings.value( "ODATA4 Update", false ).toBool();
  if ( !odata4 )
  {
    this->set_connectome_list( this->default_connectome_nicknames_, this->default_connectomes_ );
    this->set_last_connectome( 0 );
    this->settings.setValue( "ODATA4 Update", true );
  }
}

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
  QList<QVariant> list = this->settings.value( "Connectomes", this->default_connectomes_ ).toList();

  QStringList string_list;
  foreach( QVariant var, list ) {
    string_list << var.toString();
  }
  return string_list;
}

//-----------------------------------------------------------------------------
void Preferences::set_connectome_list( QStringList nicknames, QStringList list )
{
  this->settings.setValue( "Connectomes", list );
  this->settings.setValue( "Connectome Nicknames", nicknames );
  emit preferences_changed();
}

//-----------------------------------------------------------------------------
int Preferences::get_last_connectome()
{
  return this->settings.value( "LastConnectome", 0 ).toInt();
}

//-----------------------------------------------------------------------------
QStringList Preferences::get_connectome_nickname_list()
{
  QList<QVariant> list = this->settings.value( "Connectome Nicknames", this->default_connectome_nicknames_ ).toList();

  QStringList string_list;
  foreach( QVariant var, list ) {
    string_list << var.toString();
  }
  return string_list;
}

//-----------------------------------------------------------------------------
void Preferences::set_last_connectome( int id )
{
  this->settings.setValue( "LastConnectome", id );
  emit preferences_changed();
}

//-----------------------------------------------------------------------------
double Preferences::get_child_scale()
{
  return this->settings.value( "ChildScale", 1.0 ).toDouble();
}

//-----------------------------------------------------------------------------
void Preferences::set_child_scale( double scale )
{
  this->settings.setValue( "ChildScale", scale );
}

//-----------------------------------------------------------------------------
void Preferences::restore_defaults()
{
  this->set_connectome_list( this->default_connectome_nicknames_, this->default_connectomes_ );
  this->set_last_connectome( 0 );
  this->set_child_scale( 1.0 );
}
