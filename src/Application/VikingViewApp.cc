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

  file = new QFile( "C:\\Users\\amorris\\json.txt" );

  if ( !file->open( QIODevice::ReadOnly ) )
  {
    QMessageBox::information( this, tr( "HTTP" ),
                              tr( "Unable to save the file %1: %2." )
                              .arg( "asdf" ).arg( file->errorString() ) );
    delete file;
    file = 0;
    return;
  }

  //QByteArray qb = file->readAll();

  this->import_json( file->readAll() );

  return;

  QUrl url = QString("http://connectomes.utah.edu/Rabbit/OData/ConnectomeData.svc/Locations/?$filter=ParentID eq 476");

  QNetworkRequest request = QNetworkRequest( url );

  //request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

  //request.setRawHeader("Content-Type", "application/json");
  request.setRawHeader( "Accept", "application/json" );

  reply = qnam.get( request );

  file = new QFile( "C:\\Users\\amorris\\json.txt" );

  if ( !file->open( QIODevice::WriteOnly ) )
  {
    QMessageBox::information( this, tr( "HTTP" ),
                              tr( "Unable to save the file %1: %2." )
                              .arg( "asdf" ).arg( file->errorString() ) );
    delete file;
    file = 0;
    return;
  }

  this->httpRequestAborted = false;

  connect( reply, SIGNAL( finished() ),
           this, SLOT( httpFinished() ) );
  connect( reply, SIGNAL( readyRead() ),
           this, SLOT( httpReadyRead() ) );
  connect( reply, SIGNAL( downloadProgress( qint64, qint64 ) ),
           this, SLOT( updateDataReadProgress( qint64, qint64 ) ) );
}

void VikingViewApp::httpFinished()
{
  std::cerr << "http finished!\n";

/*
   QString json_text = reply->readAll();

   std::cerr << "json size = " << json_text.size() << "\n";

   QMap<QString, QVariant> map = Json::decode( json_text );

   std::cerr << "parsed " << map.size() << " elements\n";
   foreach( QString key, map.keys() ) {
    std::cerr << key.toStdString() << " => " << map.value( key ).toString().toStdString() << '\n';
   }

   QList<QVariant> list = map["value"].toList();

   QString locations = map["value"].toString();

   std::cerr << "locations.size = " << locations.size() << "\n";

   map = Json::decode( locations );

   std::cerr << "parsed " << map.size() << " elements\n";
   foreach( QString key, map.keys() ) {
    std::cerr << key.toStdString() << " => " << map.value( key ).toString().toStdString() << '\n';
   }
 */

  if ( file )
  {
    file->write( reply->readAll() );
  }

  if ( httpRequestAborted )
  {
    if ( file )
    {
      file->close();
      file->remove();
      delete file;
      file = 0;
    }
    reply->deleteLater();
#ifndef Q_WS_MAEMO_5
//    progressDialog->hide();
#endif
    return;
  }

#ifndef Q_WS_MAEMO_5
  //progressDialog->hide();
#endif
  file->flush();
  file->close();

  QVariant redirectionTarget = reply->attribute( QNetworkRequest::RedirectionTargetAttribute );
  if ( reply->error() )
  {
    file->remove();
    QMessageBox::information( this, tr( "HTTP" ),
                              tr( "Download failed: %1." )
                              .arg( reply->errorString() ) );
    //downloadButton->setEnabled(true);
  }
  else if ( !redirectionTarget.isNull() )
  {
    //QUrl newUrl = url.resolved(redirectionTarget.toUrl());
    //if (QMessageBox::question(this, tr("HTTP"),
    //  tr("Redirect to %1 ?").arg(newUrl.toString()),
    //  QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
    //    url = newUrl;
    //    reply->deleteLater();
    //    file->open(QIODevice::WriteOnly);
    //    file->resize(0);
    //    startRequest(url);
    //    return;
    //}
  }
  else
  {
    //QString fileName = QFileInfo(QUrl(urlLineEdit->text()).path()).fileName();
    //statusLabel->setText(tr("Downloaded %1 to %2.").arg(fileName).arg(QDir::currentPath()));
    //downloadButton->setEnabled(true);
  }

  reply->deleteLater();
  reply = 0;
  delete file;
  file = 0;
}

void VikingViewApp::httpReadyRead()
{
  std::cerr << "http read...\n";
  // this slot gets called every time the QNetworkReply has new data.
  // We read all of its new data and write it into the file.
  // That way we use less RAM than when reading it at the finished()
  // signal of the QNetworkReply
  //if (file)
  //file->write(reply->readAll());
}

void VikingViewApp::updateDataReadProgress( qint64 bytesRead, qint64 totalBytes )
{
  //std::cerr << "http update...\n";

  if ( httpRequestAborted )
  {
    return;
  }

#ifndef Q_WS_MAEMO_5
//  progressDialog->setMaximum(totalBytes);
//progressDialog->setValue(bytesRead);
#else
  Q_UNUSED( bytesRead );
  Q_UNUSED( totalBytes );
#endif
}

void VikingViewApp::import_json( QString json_text )
{
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
