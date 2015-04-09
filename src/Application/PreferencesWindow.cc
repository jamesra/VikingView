/*
 * Shapeworks license
 */

// qt
#include <QtGui>

#include <Application/PreferencesWindow.h>
#include <Application/Preferences.h>

// The interface from the designer
#include "ui_PreferencesWindow.h"

//-----------------------------------------------------------------------------
PreferencesWindow::PreferencesWindow( QWidget* parent /*= 0 */ )
{
  this->ui_ = new Ui_PreferencesWindow;
  this->ui_->setupUi( this );


  QPushButton* reset_button = this->ui_->button_box->button( QDialogButtonBox::RestoreDefaults );
  QObject::connect( reset_button, SIGNAL( clicked() ), this, SLOT( restore_defaults() ) );
}


//-----------------------------------------------------------------------------
void PreferencesWindow::restore_defaults()
{
  Preferences::Instance().restore_defaults();
  this->set_values_from_preferences();
}

//-----------------------------------------------------------------------------
void PreferencesWindow::set_values_from_preferences()
{

}

//-----------------------------------------------------------------------------
void PreferencesWindow::on_add_connectome_button_clicked()
{

}

//-----------------------------------------------------------------------------
void PreferencesWindow::on_delete_connectome_button_clicked()
{

}

