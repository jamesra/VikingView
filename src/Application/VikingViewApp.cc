// std
#include <iostream>

// qt
#include <QFileDialog>
#include <QWidgetAction>
#include <QInputDialog>
#include <QMessageBox>
#include <QUrl>

// vtk
#include <vtkRenderWindow.h>

// viking
#include <Application/VikingViewApp.h>
#include <Data/Json.h>
#include <Data/PointSampler.h>
#include <Data/AlphaShape.h>
#include <Data/Downloader.h>
#include <Visualization/Viewer.h>

// ui
#include <ui_VikingViewApp.h>

//---------------------------------------------------------------------------
VikingViewApp::VikingViewApp( int argc, char** argv )
{
  this->ui_ = new Ui_VikingViewApp;
  this->ui_->setupUi( this );

  this->viewer_ = new Viewer();

  this->viewer_->set_render_window( this->ui_->qvtkWidget->GetRenderWindow() );
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

    int id = text.toInt();
    std::cerr << "id = " << id << "\n";
    //textLabel->setText( text );
    this->load_structure( id );
  }
}

//---------------------------------------------------------------------------
void VikingViewApp::on_delete_button_clicked()
{}

//---------------------------------------------------------------------------
void VikingViewApp::load_structure( int id )
{

  Downloader downloader;
  QSharedPointer<Structure> structure = downloader.download_structure(id);

  PointSampler ps(structure);
  std::list<Point> points = ps.sample_points();

  AlphaShape alpha_shape;
  alpha_shape.set_points( points );
  vtkSmartPointer<vtkPolyData> poly_data = alpha_shape.get_mesh();

  this->viewer_->display_mesh( poly_data );

  return;


  QFile *file = new QFile( "C:\\Users\\amorris\\json.txt" );

  if ( !file->open( QIODevice::ReadOnly ) )
  {
    QMessageBox::information( this, tr( "HTTP" ),
                              tr( "Unable to save the file %1: %2." )
                              .arg( "asdf" ).arg( file->errorString() ) );
    delete file;
    file = 0;
    return;
  }

 
}


void VikingViewApp::import_json( QString json_text )
{
/*
  std::cerr << "json size = " << json_text.size() << "\n";

  QMap<QString, QVariant> map = Json::decode( json_text );

  std::cerr << "parsed " << map.size() << " elements\n";
  foreach( QString key, map.keys() ) {
    std::cerr << key.toStdString() << " => " << map.value( key ).toString().toStdString() << '\n';
  }

  QList<QVariant> list = map["value"].toList();

  PointSampler ps;
  ps.set_locations( list );
  std::list<Point> points = ps.sample_points();

  AlphaShape alpha_shape;
  alpha_shape.set_points( points );
  vtkSmartPointer<vtkPolyData> poly_data = alpha_shape.get_mesh();

  this->viewer_->display_mesh( poly_data );
*/



//vtk

/*
   int count = 0;
   foreach( QVariant var, list ) {
    count++;
    if ( count < 5 )
    {
      QMap<QString, QVariant> item = var.toMap();
      foreach( QString key, item.keys() ) {
        std::cerr << key.toStdString() << " => " << item.value( key ).toString().toStdString() << '\n';
      }
    }
   }
 */
/*
   QString locations = map["value"].toString();

   std::cerr << "locations.size = " << locations.size() << "\n";

   map = Json::decode(locations);

   std::cerr << "parsed " << map.size() << " elements\n";
   foreach( QString key, map.keys() )
   {
    std::cerr << key.toStdString() << " => " << map.value( key ).toString().toStdString() << '\n';
   }
 */
}
