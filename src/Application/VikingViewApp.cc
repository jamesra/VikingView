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

  this->ui_->connectome_combo->view()->setMinimumWidth(400);

  this->ui_->sampling_slider->hide();
  this->ui_->sampling_label->hide();
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
      std::cerr << "id = " << id << "\n";
      //textLabel->setText( text );
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

  QProgressDialog progress( "Downloading...", "Abort", 0, 4, this );
  progress.setWindowModality( Qt::WindowModal );
  progress.setMinimumDuration( 500 );

  progress.setValue(0);
  Downloader downloader;
  QSharedPointer<Structure> structure = downloader.download_structure( id );

  progress.setLabelText("Generating Mesh...");
  this->structures_.append( structure );
  progress.setValue(2);

  this->viewer_->display_structures( this->structures_ );
  progress.setValue(4);

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

  for ( int i = 0; i < this->structures_.size(); i++ )
  {
    QSharedPointer<Structure> structure = this->structures_[i];

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
  this->viewer_->set_clipping_plane(this->ui_->cutting_plane_button->isChecked());
}
