// std
#include <iostream>

// qt
#include <QFileDialog>
#include <QWidgetAction>
#include <QInputDialog>
#include <QMessageBox>
#include <QUrl>
#include <QProgressDialog>

// vtk
#include <vtkRenderWindow.h>

// viking
#include <Application/VikingViewApp.h>
#include <Application/Preferences.h>
#include <Data/Json.h>
#include <Data/PointSampler.h>
#include <Data/AlphaShape.h>
#include <Data/Downloader.h>
#include <Data/Structure.h>
#include <Visualization/Viewer.h>

// ui
#include <ui_VikingViewApp.h>

//---------------------------------------------------------------------------
VikingViewApp::VikingViewApp( int argc, char** argv )
{
  this->ui_ = new Ui_VikingViewApp;
  this->ui_->setupUi( this );

  //this->ui_->connectome_combo->view()->setMinimumWidth( 400 );

  this->ui_->sampling_slider->hide();
  this->ui_->sampling_label->hide();

  // resize from preferences
  this->resize( Preferences::Instance().get_main_window_size() );

  connect( &( Preferences::Instance() ), SIGNAL( preferences_changed() ), this, SLOT( on_preferences_changed() ) );
  this->on_preferences_changed();
}

//---------------------------------------------------------------------------
VikingViewApp::~VikingViewApp()
{}

//---------------------------------------------------------------------------
void VikingViewApp::on_add_button_clicked()
{
  bool ok;
  QString text = QInputDialog::getText( this, tr( "Please enter an ID or list of IDs" ),
                                        tr( "ID:" ), QLineEdit::Normal,
                                        "", &ok );
  if ( ok && !text.isEmpty() )
  {

    QStringList pieces = text.split( " " );

    foreach( QString str, pieces ) {
      int id = str.toInt();
      this->load_structure( id );
    }
  }
}

//---------------------------------------------------------------------------
void VikingViewApp::on_delete_button_clicked()
{
  QModelIndexList list = this->ui_->table_widget->selectionModel()->selectedRows();

  QList<int> index_list;

  for ( int i = list.size() - 1; i >= 0; i-- )
  {
    index_list << list[i].row();
  }

  foreach( int i, index_list ) {
    this->structures_.erase( this->structures_.begin() + i );
  }

  this->viewer_->display_structures( this->structures_ );
  this->update_table();
}

//---------------------------------------------------------------------------
void VikingViewApp::load_structure( int id )
{

  QProgressDialog progress( "Downloading...", "Abort", 0, 3, this );
  progress.setWindowModality( Qt::WindowModal );
  progress.setMinimumDuration( 500 );

  progress.setValue( 0 );
  Downloader downloader;

  QString end_point = Preferences::Instance().get_connectome_list()[this->ui_->connectome_combo->currentIndex()];

  DownloadObject download_object;
  downloader.download_cell2( end_point, id, download_object );

  progress.setLabelText( "Generating Mesh..." );
  progress.setValue( 1 );
/*

   foreach( QVariant var, download_object.structure_list )
   {
    QMap<QString, QVariant> item = var.toMap();
    int sid = item["ID"].toLongLong();
    std::cerr << "sid = " << sid << "\n";

    QSharedPointer<Structure> structure = Structure::create_structure( sid, download_object.structure_list,
      download_object.location_list, download_object.link_list );
    this->structures_[sid] = structure;

    }

 */

  QSharedPointer<StructureHash> structures = Structure::create_structures( download_object.structure_list,
    download_object.location_list, 
    download_object.link_list );

  //QSharedPointer<Structure> structure = Structure::create_structure( id, download_object.structure_list,
  //  download_object.location_list, 
  //  download_object.link_list );


  foreach (QSharedPointer<Structure> structure, structures->values())
  {

    this->structures_[structure->get_id()] = structure;

  //  if ( structure->get_node_map().size() == 0 )
  //  {
  //    progress.close();
  //    QMessageBox::critical( NULL, "VikingView", "Error: Download error or Invalid structure", QMessageBox::Ok );
  //    return;
  //  }

  }

  progress.setValue( 2 );

  this->viewer_->display_structures( this->structures_ );
  progress.setValue( 3 );

  this->update_table();

  this->viewer_->redraw();

  return;
}

//---------------------------------------------------------------------------
void VikingViewApp::update_table()
{
  this->ui_->table_widget->clear();

  this->ui_->table_widget->setRowCount( this->structures_.size() );
  this->ui_->table_widget->setColumnCount( 3 );

  QStringList table_header;
  table_header << "Id" << "Volume" << "Center of mass";
  this->ui_->table_widget->setHorizontalHeaderLabels( table_header );

  this->ui_->table_widget->verticalHeader()->setVisible( false );

  foreach( int i, this->structures_.keys() ) {
    QSharedPointer<Structure> structure = this->structures_[i];
    //std::cerr << "s = " << i << "\n";

    QTableWidgetItem* new_item = new QTableWidgetItem( QString::number( this->structures_[i]->get_id() ) );
    this->ui_->table_widget->setItem( i, 0, new_item );

    new_item = new QTableWidgetItem( QString::number( this->structures_[i]->get_volume() ) );
    this->ui_->table_widget->setItem( i, 1, new_item );

    new_item = new QTableWidgetItem( this->structures_[i]->get_center_of_mass_string() );
    this->ui_->table_widget->setItem( i, 2, new_item );
  }

  this->ui_->table_widget->resizeColumnsToContents();
  //this->ui_->table_widget->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
  this->ui_->table_widget->horizontalHeader()->setStretchLastSection( true );

  this->ui_->table_widget->setSelectionBehavior( QAbstractItemView::SelectRows );
}

//---------------------------------------------------------------------------
void VikingViewApp::on_opacity_slider_valueChanged()
{
//  std::cerr << "opacity now : " << this->ui_->opacity_slider->value() << "\n";
  this->viewer_->set_opacity( this->ui_->opacity_slider->value() / 100.0 );
}

//---------------------------------------------------------------------------
void VikingViewApp::on_sampling_slider_valueChanged()
{
//  std::cerr << "sampling now : " << this->ui_->sampling_slider->value() << "\n";
}

//---------------------------------------------------------------------------
void VikingViewApp::on_action_quit_triggered()
{
  this->close();
}

//---------------------------------------------------------------------------
void VikingViewApp::on_action_preferences_triggered()
{
  Preferences::Instance().show_window();
}

//---------------------------------------------------------------------------
void VikingViewApp::initialize_vtk()
{
  this->viewer_ = new Viewer();
  this->viewer_->set_render_window( this->ui_->qvtkWidget->GetRenderWindow() );
  this->viewer_->redraw();
}

//---------------------------------------------------------------------------
void VikingViewApp::on_auto_view_button_clicked()
{
  this->viewer_->reset_camera();
  this->viewer_->redraw();
}

//---------------------------------------------------------------------------
void VikingViewApp::on_cutting_plane_button_clicked()
{
  this->viewer_->set_clipping_plane( this->ui_->cutting_plane_button->isChecked() );
}

//---------------------------------------------------------------------------
void VikingViewApp::closeEvent( QCloseEvent* event )
{
  // close the preferences window in case it is open
  Preferences::Instance().close_window();

  // save the size of the window to preferences
  Preferences::Instance().set_main_window_size( this->size() );

  // save the last used connectome
  Preferences::Instance().set_last_connectome( this->ui_->connectome_combo->currentIndex() );
  this->ui_->connectome_combo->setCurrentIndex( Preferences::Instance().get_last_connectome() );
}

//---------------------------------------------------------------------------
void VikingViewApp::on_preferences_changed()
{

  this->ui_->connectome_combo->clear();
  this->ui_->connectome_combo->addItems( Preferences::Instance().get_connectome_nickname_list() );
  int last_connectome = Preferences::Instance().get_last_connectome();
  if ( last_connectome < 0 || last_connectome >= this->ui_->connectome_combo->count() )
  {
    last_connectome = 0;
  }
  this->ui_->connectome_combo->setCurrentIndex( last_connectome );
}

//---------------------------------------------------------------------------
void VikingViewApp::on_connectome_configure_clicked()
{
  Preferences::Instance().show_window();
}
