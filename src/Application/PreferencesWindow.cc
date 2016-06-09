/*
 * Shapeworks license
 */

// qt
#include <QtGui>
#include <QLineEdit>
#include <QInputDialog>
#include <QStringList>
#include <QPushButton>

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

  QPushButton* ok_button = this->ui_->button_box->button( QDialogButtonBox::Ok );
  QObject::connect( ok_button, SIGNAL( clicked() ), this, SLOT( close() ) );

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

  QStringList nicknames = Preferences::Instance().get_connectome_nickname_list();
  QStringList urls = Preferences::Instance().get_connectome_list();

  if (nicknames.size() != urls.size())
  {
    Preferences::Instance().restore_defaults();
    nicknames = Preferences::Instance().get_connectome_nickname_list();
    urls = Preferences::Instance().get_connectome_list();
  }

  this->ui_->connectome_table->clear();

  this->ui_->connectome_table->setRowCount( nicknames.size() );
  this->ui_->connectome_table->setColumnCount( 2 );

  QStringList table_header;
  table_header << "Nickname" << "URL";
  this->ui_->connectome_table->setHorizontalHeaderLabels( table_header );

  this->ui_->connectome_table->verticalHeader()->setVisible( false );

  for ( int i = 0; i < nicknames.size(); i++ )
  {

    QTableWidgetItem* new_item = new QTableWidgetItem( nicknames[i] );
    this->ui_->connectome_table->setItem( i, 0, new_item );

    new_item = new QTableWidgetItem( urls[i] );
    this->ui_->connectome_table->setItem( i, 1, new_item );
  }

  this->ui_->connectome_table->resizeColumnsToContents();
  this->ui_->connectome_table->horizontalHeader()->setStretchLastSection( true );
  this->ui_->connectome_table->setSelectionBehavior( QAbstractItemView::SelectRows );
}

//-----------------------------------------------------------------------------
void PreferencesWindow::on_add_connectome_button_clicked()
{
  bool ok;
  QString text = QInputDialog::getText( this, tr( "Please enter an endpoint" ),
                                        tr( "Endpoint:" ), QLineEdit::Normal, "", &ok );
  if ( !ok || text.isEmpty() )
  {
    return;
  }

  QString nickname = QInputDialog::getText( this, tr( "Please enter a nickname for this endpoint" ),
                                            tr( "Nickname:" ), QLineEdit::Normal, "", &ok );
  if ( !ok || nickname.isEmpty() )
  {
    return;
  }

  QStringList list = Preferences::Instance().get_connectome_list();
  list << text;

  QStringList nicknames = Preferences::Instance().get_connectome_nickname_list();
  nicknames << nickname;

  Preferences::Instance().set_connectome_list( nicknames, list );

  this->set_values_from_preferences();
}

//-----------------------------------------------------------------------------
void PreferencesWindow::on_delete_connectome_button_clicked()
{

  QModelIndexList list = this->ui_->connectome_table->selectionModel()->selectedRows();

  QList<int> index_list;

  for ( int i = list.size() - 1; i >= 0; i-- )
  {
    index_list << list[i].row();
  }

  QStringList connectome_list = Preferences::Instance().get_connectome_list();
  QStringList nicknames = Preferences::Instance().get_connectome_nickname_list();

  foreach( int i, index_list ) {
    connectome_list.removeAt( i );
    nicknames.removeAt( i );
  }
  Preferences::Instance().set_connectome_list( nicknames, connectome_list );
  this->set_values_from_preferences();
}
