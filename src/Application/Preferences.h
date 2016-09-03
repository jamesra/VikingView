#ifndef VIKINGVIEW_APPLICATION_PREFERENCES_H
#define VIKINGVIEW_APPLICATION_PREFERENCES_H

#include <QSettings>
#include <QString>
#include <Application/PreferencesWindow.h>

//! Application preferences
/*!
 * The Preferences singleton controls all preferences for the application.
 * Values persist via the QSettings class.
 */
class Preferences : public QObject
{
  Q_OBJECT;

public:

  /// get the singleton instance
  static Preferences& Instance();

  /// show the PreferencesWindow
  void show_window();

  /// close the PreferencesWindow
  void close_window();

  //--- general preferences

  /// Main window size
  QSize get_main_window_size();
  void set_main_window_size( QSize size );

  QStringList get_connectome_list();
  void set_connectome_list( QStringList nicknames, QStringList list );

  void add_connectome(QString nickname, QString endpoint);

  QStringList get_connectome_nickname_list();

  int get_last_connectome();
  void set_last_connectome( int id );

  int get_active_endpoint() { return iActiveEndpoint; };

  QString active_endpoint() { return default_connectomes_[iActiveEndpoint]; }

  double get_child_scale();
  void set_child_scale( double scale );

  /// restore all default values
  void restore_defaults();

public Q_SLOTS: 
  void set_active_endpoint(int id);
   
Q_SIGNALS:
  //void color_scheme_changed( int newIndex );
  //void glyph_properties_changed();
  void preferences_changed();
  void active_endpoint_changed();

private:

  /// private constructor
  Preferences();

  /// stop the compiler generating methods of copy the object
  Preferences( Preferences const& copy );            // not implemented
  Preferences& operator=( Preferences const& copy ); // not implemented

  /// singleton
  static Preferences instance;

  /// the QSettings object
  QSettings settings;

  /// the preferences user interface
  PreferencesWindow preferences_window;

  QStringList default_connectomes_;
  QStringList default_connectome_nicknames_;

  int iActiveEndpoint = 0;
};

#endif // ifndef VIKINGVIEW_APPLICATION_PREFERENCES_H
