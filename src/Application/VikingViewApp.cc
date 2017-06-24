// std
#include <iostream>

// qt
#include <QFileDialog>
#include <QWidgetAction>
#include <QInputDialog>
#include <QMessageBox>
#include <QUrl>
#include <QProgressDialog>
#include <QXmlStreamWriter>
#include <QDateTime>

// vtk
#include <vtkRenderWindow.h>
#include <vtkOBJExporter.h>

// viking
#include <Application/VikingViewApp.h>
#include <Application/Preferences.h>
#include <Application/ModelController.h>
#include <Data/Json.h>
//#include <Data/PointSampler.h>
//#include <Data/AlphaShape.h>
#include <Data/Downloader.h>
#include <Data/Structure.h>
#include <Visualization/Viewer.h>
#include <Application/ModelController.h>
#include <Application/ChooseStructuresDialog.h>

// ui
#include <ui_VikingViewApp.h>

//---------------------------------------------------------------------------
VikingViewApp::VikingViewApp( QSharedPointer< CommandLineArgs > command_line_args)
  : command_line_args_( command_line_args )
{
  this->viewer_ = NULL;
  this->ui_ = new Ui_VikingViewApp;
  this->ui_->setupUi( this );
  
  //this->ui_->connectome_combo->view()->setMinimumWidth( 400 );

  this->ui_->sampling_slider->hide();
  this->ui_->sampling_label->hide();

  // resize from preferences
  this->resize( Preferences::Instance().get_main_window_size() );

  this->setAcceptDrops(true);
  /*this->ui_->table_widget->setAcceptDrops(true);
  this->ui_->centralwidget->setAcceptDrops(true);
  
  bool dropsOK = this->ui_->centralwidget->acceptDrops();
  */
  bool dropsOK = this->acceptDrops();
  
  connect( &( Preferences::Instance() ), SIGNAL( preferences_changed() ), this, SLOT( on_preferences_changed() ) );
  connect(this->ui_->connectome_combo, SIGNAL(currentIndexChanged(int)), &(Preferences::Instance()), SLOT(set_active_endpoint(int)) );
  /*
  connect(this->ui_->centralwidget, SIGNAL(dragEnterEvent(QDragEnterEvent *)), this, SLOT(dragEnterEvent(QDragEnterEvent *)));
  connect(this->ui_->centralwidget, SIGNAL(dropEvent(QDropEvent *)), this, SLOT(dropEvent(QDropEvent *)));
  connect(this->ui_->table_widget, SIGNAL(dragEnterEvent(QDragEnterEvent *)), this, SLOT(dragEnterEvent(QDragEnterEvent *)));
  connect(this->ui_->table_widget, SIGNAL(dropEvent(QDropEvent *)), this, SLOT(dropEvent(QDropEvent *)));
  */
  this->on_preferences_changed();

  this->initialize_export_functions();

}

//---------------------------------------------------------------------------
VikingViewApp::~VikingViewApp()
{}

void VikingViewApp::dragEnterEvent(QDragEnterEvent *event)
{
	if (event->mimeData()->hasFormat("text/plain"))
		event->acceptProposedAction();

	if (event->mimeData()->hasText())
		event->acceptProposedAction();

	if (event->mimeData()->hasUrls())
		event->acceptProposedAction();
}

void VikingViewApp::dragMoveEvent(QDragMoveEvent* event)
{
	// if some actions should not be usable, like move, this code must be adopted
	event->acceptProposedAction();
}

void VikingViewApp::dropEvent(QDropEvent * event)
{
	QList<long> ids;

	if (event->mimeData()->hasUrls())
	{
		QList<QUrl> StructureIDs = event->mimeData()->urls();
		foreach(QUrl url, StructureIDs)
		{
			if(url.isLocalFile())
				ids.append(ParseInputArray(url.toLocalFile()));
			else
				ids.append(ParseInputArray(url.toString()));
		}
	}
	else if (event->mimeData()->hasText())
	{
		QString StructureIDs = event->mimeData()->text();
		ids = ParseInputArray(StructureIDs);
	}
	

	this->load_structures(ids);

	event->acceptProposedAction();
}

//---------------------------------------------------------------------------
void VikingViewApp::on_add_button_clicked()
{
  bool ok;
  QString text = ChooseStructuresDialog::getStructureIDs(this, tr("StructureID input"), "", &ok);
  //QString text = QInputDialog::getText( this, tr( "Please enter an ID or list of IDs" ),
    //                                    tr( "ID:" ), QLineEdit::Normal,
      //                                  "", &ok );
  if ( ok && !text.isEmpty() )
  {
	QList<long> ids = ParseInputArray(text);
	this->load_structures(ids);
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
    this->cells_.erase( this->cells_.begin() + i );
  }

//  this->viewer_->display_structures( this->structures_ );
  this->viewer_->display_cells( this->cells_, false );
  this->update_table();
} 

//---------------------------------------------------------------------------
void VikingViewApp::load_structures(QList<long> ids)
{
	if (ids.count() == 0)
		return;

	QSharedPointer<QProgressDialog> progressDialog = QSharedPointer<QProgressDialog>( new QProgressDialog("Downloading Scale...", "Abort", 0, 100, this));
	progressDialog->setWindowModality(Qt::WindowModal);
	progressDialog->setMinimumDuration(500);
	progressDialog->setValue(0);
	progressDialog->setAutoClose(false);

	WindowedProgressReporter progressReporter = WindowedProgressReporter(progressDialog);
	  
	ColorMapper cmap = ColorMapper("./StructureTypeColors.txt", "./StructureColors.txt");
	QString end_point = Preferences::Instance().get_connectome_list()[this->ui_->connectome_combo->currentIndex()];

	
	  
	QList<QSharedPointer<Structure> > new_cells = LoadStructures(ids, end_point, cmap, progressReporter);
	
	foreach(QSharedPointer<Structure> new_cell, new_cells)
	{
		this->cells_ << new_cell;

		foreach(QSharedPointer<Structure> structure, new_cell->structures.values()) {
			this->structures_[structure->get_id()] = structure;
		}
	}

	progressReporter.set_min(0);
	progressReporter.set_min(100);
	progressReporter(50, "Generating Mesh...");
	//progress.setLabelText("Generating Mesh...");
	//progress.setValue( 3  );

	  //this->viewer_->display_structures( this->structures_ );
	  this->viewer_->display_cells( this->cells_, true );

	  progressReporter(95, "Updating UI...");
//	  progress.setLabelText("Updating UI...");
//	  progress.setValue( 4 );

	  this->update_table();

	  this->viewer_->redraw();


	  return;
}

vtkRenderWindow* VikingViewApp::get_render_window()
{
  return this->ui_->qvtkWidget->GetRenderWindow();
}

//---------------------------------------------------------------------------
void VikingViewApp::update_table()
{
  std::cerr << "Updating the table" << std::endl;
  this->ui_->table_widget->clear();

  this->ui_->table_widget->setRowCount( this->structures_.size() );
  this->ui_->table_widget->setColumnCount( 3 );

  QStringList table_header;
  table_header << "Id" << "Volume" << "Center of mass";
  this->ui_->table_widget->setHorizontalHeaderLabels( table_header );

  this->ui_->table_widget->verticalHeader()->setVisible( false );

  for ( int i = 0; i < this->cells_.size(); i++ )
  {

    QTableWidgetItem* new_item = new QTableWidgetItem( QString::number( this->cells_[i]->get_id() ) );
    this->ui_->table_widget->setItem( i, 0, new_item );

    //new_item = new QTableWidgetItem( QString::number( this->structures_[i]->get_volume() ) );
    //this->ui_->table_widget->setItem( i, 1, new_item );

    //new_item = new QTableWidgetItem( this->structures_[i]->get_center_of_mass_string() );
    //this->ui_->table_widget->setItem( i, 2, new_item );
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
  this->viewer_ = new Viewer( command_line_args_ );
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
  if (last_connectome < 0 || last_connectome >= this->ui_->connectome_combo->count())
  {
	  last_connectome = 0;
  }
  this->ui_->connectome_combo->setCurrentIndex(last_connectome);

  this->ui_->child_scale->setValue(Preferences::Instance().get_child_scale());
}

//---------------------------------------------------------------------------
void VikingViewApp::on_connectome_configure_clicked()
{
  Preferences::Instance().show_window();
}

//---------------------------------------------------------------------------
void VikingViewApp::on_child_scale_valueChanged(double scale)
{
  //double scale = this->ui_->child_scale->value();
  Preferences::Instance().set_child_scale(scale);

  if (this->viewer_)
  {
    this->viewer_->display_cells(this->cells_, false);
  }
}

void VikingViewApp::initialize_export_functions()
{
  this->export_functions_[ "dae" ] = &VikingViewApp::export_dae;
  this->export_functions_[ "obj" ] = &VikingViewApp::export_obj;
}

void VikingViewApp::export_cell( QString filename, QString export_type )
{
  ExportFunction export_function = this->export_functions_[ export_type ];
  (this->*export_function)( filename );
}

//---------------------------------------------------------------------------
void VikingViewApp::export_dae(QString filename)
{
  std::cerr << "exporting dae to \"" << filename.toStdString() << "\"\n";

  // Give a warning that dae export is unfinished
  QMessageBox::warning(0, "export dae not implemented", "The requested cell export function is not yet fully implemented, so the resulting file will not appear as expected.");

  // open file
  QFile file(filename + ".xml");

  if (!file.open(QIODevice::WriteOnly))
  {
	QMessageBox::critical(0, "Read only", "The file is in read only mode");
    return;
  }

  // -id 180 -export "z:/shared/file.dae"

  // setup XML
  QSharedPointer<QXmlStreamWriter> xml = QSharedPointer<QXmlStreamWriter>(new QXmlStreamWriter());
  xml->setAutoFormatting(true);
  xml->setDevice(&file);
  xml->writeStartDocument();

  // write the root element
  xml->writeStartElement("COLLADA");
  xml->writeAttribute("xmlns", "http://www.collada.org/2005/11/COLLADASchema");
  xml->writeAttribute("version", "1.4.1");

  xml->writeStartElement("asset");

  xml->writeStartElement("contributor");
  xml->writeTextElement("authoring_tool", "VikingView");
  xml->writeEndElement(); // contributor

  xml->writeTextElement("created", QDateTime::currentDateTime().toUTC().toString(Qt::ISODate) + "Z");
  xml->writeTextElement("modified", QDateTime::currentDateTime().toUTC().toString(Qt::ISODate) + "Z");

  xml->writeStartElement("unit");
  xml->writeAttribute("meter", "1");
  xml->writeAttribute("name", "meter");
  xml->writeEndElement(); // unit

  xml->writeTextElement("up_axis", "Z_UP");

  xml->writeEndElement(); // asset

  xml->writeStartElement("library_geometries");
  xml->writeEndElement(); //library_geometries

  xml->writeEndElement(); // COLLADA
  xml->writeEndDocument();
}

//---------------------------------------------------------------------------
void VikingViewApp::export_obj(QString filename)
{
	// Create an OBJ file exporter
	vtkOBJExporter* vtkExporter = vtkOBJExporter::New();
	// Give the exporter access to the render window
	vtkRenderWindow* renderWindow = this->get_render_window();
	vtkExporter->SetRenderWindow(renderWindow);
	vtkExporter->SetFilePrefix(filename.toStdString().c_str());
	vtkExporter->Write();
}