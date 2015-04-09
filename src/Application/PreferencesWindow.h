#ifndef VIKINGVIEW_APPLICATION_PREFERENCES_WINDOW_H
#define VIKINGVIEW_APPLICATION_PREFERENCES_WINDOW_H

#include <QDialog>

class Ui_PreferencesWindow;
class QAbstractButton;

//! Qt UI dialog to control preferences
/*!
 * The PreferenceWindow provides controls over preferences for the
 * application.  It is synchronized with the singleton Preference object
 */
class PreferencesWindow : public QDialog
{
  Q_OBJECT

public:
  PreferencesWindow( QWidget* parent = 0 );

  void set_values_from_preferences();

public Q_SLOTS:

  void on_add_connectome_button_clicked();
  void on_delete_connectome_button_clicked();

  void restore_defaults();

private:
  Ui_PreferencesWindow* ui_;
};

#endif // VIKINGVIEW_APPLICATION_PREFERENCES_WINDOW_H
