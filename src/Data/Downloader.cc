#include <Data/Downloader.h>
#include <Data/Structure.h>
#include <Data/Json.h>

#include <QString>
#include <QEventLoop>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QtConcurrentMap>
#include <QElapsedTimer>

Downloader::Downloader()
{}

Downloader::~Downloader()
{}

//-----------------------------------------------------------------------------
bool Downloader::download_cell( QString end_point, int id, DownloadObject &download_object, QProgressDialog &progress )
{
  try{

    QElapsedTimer timer;
    timer.start();

    // set number of threads to download
    QThreadPool::globalInstance()->setMaxThreadCount( 16 );

    QString request = QString( end_point + "/Structures?$filter=(ID eq " ) + QString::number( id )
                      + " or ParentID eq " + QString::number( id ) + ")&$select=ID,TypeID";
    download_object.structure_list = this->download_json( request, QString( "structures-" ) + QString::number( id ) );

    std::cerr << "structure list length = " << download_object.structure_list.size() << "\n";

    QList< QList<QVariant> > downloaded;
    QList< QString > requests;

    foreach( QVariant var, download_object.structure_list ) {
      QMap<QString, QVariant> item = var.toMap();
      int id = item["ID"].toLongLong();
      request = QString( end_point + "/Structures(" ) + QString::number( id ) + ")/Locations?$select=ID,VolumeX,VolumeY,Z,Radius,ParentID";
      requests.append( request );
    }

    downloaded = QtConcurrent::blockingMapped( requests, download_item );
    progress.setValue( 1 );

    foreach( QList<QVariant> list, downloaded ) {
      download_object.location_list.append( list );
    }

    // download locations
    requests.clear();
    foreach( QVariant var, download_object.structure_list ) {
      QMap<QString, QVariant> item = var.toMap();
      int id = item["ID"].toLongLong();
      request = QString( end_point + "/Structures(" ) + QString::number( id ) + ")/LocationLinks?$select=A,B";
      requests.append( request );
    }

    // download location links
    downloaded = QtConcurrent::blockingMapped( requests, download_item );
    progress.setValue( 2 );
    foreach( QList<QVariant> list, downloaded ) {
      download_object.link_list.append( list );
    }

    std::cerr << "Download took: " << timer.elapsed() / 1000.0 << " seconds\n";
    return true;
  }
  catch ( DownloadException e )
  {
    std::cerr << e.message_.toStdString() << "\n";
    QMessageBox::critical( 0, "Error", e.message_ );
  }
  return false;
}

//-----------------------------------------------------------------------------
QList<QVariant> Downloader::download_json( QString url_string, QString file_prefix )
{
  //std::cerr << "download_json(" << url_string.toStdString() << "\n";
  const int save_to_file = 0;
  const int load_from_file = 0;

  QList<QVariant> list;

  if ( load_from_file )
  {
    QFile* file = new QFile( "/tmp/json-" + file_prefix + ".txt" );

    if ( !file->open( QIODevice::ReadOnly ) )
    {
      std::cerr << "Error opening file for reading\n";
    }

    QTextStream ts( file );

    QMap<QString, QVariant> map = Json::decode( ts.readAll() );

    QList<QVariant> this_list = map["value"].toList();
    list.append( this_list );

    /*
       int num_pages = ts.readLine().toInt();
       for ( int i = 0; i < num_pages; i++ )
       {
       QString qs = ts.readLine();

          QMap<QString, QVariant> map = Json::decode( qs );

          QList<QVariant> this_list = map["value"].toList();
          list.append( this_list );
       }
     */

    file->close();

    return list;
  }

  QList<QString> pages;

  bool more = false;

  do
  {

    QString text = Downloader::download_url( url_string );
    pages.append( text );

    QMap<QString, QVariant> map = Json::decode( text );

    if ( !map.contains( "value" ) )
    {
      QString message = "Error downloading url: " + url_string + "\n\n" + text.left( 256 );
      throw DownloadException( message );
    }

    list.append( map["value"].toList() );

    more = map.contains( "odata.nextLink" );

    if ( more )
    {
      QString link = map["odata.nextLink"].toString();
      QString next_page;
      if ( link.contains( ".svc" ) )
      {
        next_page = url_string.split( ".svc" )[0] + ".svc" + link.split( ".svc" )[1];
      }
      else
      {
        next_page = url_string.split( ".svc" )[0] + ".svc/" + link;
      }
      url_string = next_page;
      std::cerr << "next page? " << next_page.toStdString() << "\n";
    }
  }
  while ( more );

  if ( save_to_file )
  {
    QFile* file = new QFile( "/tmp/json-" + file_prefix + ".txt" );

    if ( !file->open( QIODevice::WriteOnly ) )
    {
      std::cerr << "Error opening file for writing\n";
    }

    QTextStream ts( file );
//    ts << pages.size() << "\n";
//    for ( int i = 0; i < pages.size(); i++ )
//    {
//      ts << pages[i] << "\n";
//    }

    ts << pages[0];

    file->close();
  }

  return list;
}

//-----------------------------------------------------------------------------
QList<QVariant> Downloader::download_item( QString request )
{
  QList<QVariant> list = Downloader::download_json( request, QString( "json-" ) + request );
  return list;
}

//-----------------------------------------------------------------------------
QString Downloader::download_url( QString url_string )
{
  QNetworkAccessManager qnam;

  QUrl url = url_string;
  QNetworkRequest request = QNetworkRequest( url );
  request.setRawHeader( "Accept", "application/json" );
  QNetworkReply* reply = qnam.get( request );

  // wait for download
  QEventLoop loop;
  connect( reply, SIGNAL( finished() ), &loop, SLOT( quit() ) );
  loop.exec();

  QString text = reply->readAll();

  return text;
}
