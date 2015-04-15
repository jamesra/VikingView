#include <Data/Downloader.h>
#include <Data/Structure.h>
#include <Data/Json.h>

#include <QString>
#include <QEventLoop>
#include <QFile>
#include <QTextStream>

Downloader::Downloader()
{}

Downloader::~Downloader()
{}

//-----------------------------------------------------------------------------
bool Downloader::download_cell( QString end_point, int id, DownloadObject &download_object )
{
  try{

    QString request = QString( end_point + "/Structures/?$filter=ParentID eq " )
                      + QString::number( id ) + " or ID eq " + QString::number( id );
    download_object.structure_list = this->download_json( request, QString( "structures-" ) + QString::number( id ) );

    //request = QString( end_point + "/Locations/?$filter=ParentID eq " ) + QString::number( id );
    request = QString( end_point + "/SelectStructureLocations?ID=" ) + QString::number( id ) + "L" + "&$format=json";
    download_object.location_list = this->download_json( request, QString( "locations-" ) + QString::number( id ) );

    //request = QString( end_point + "/SelectStructureLocationLinks?StructureID=" ) + QString::number( id ) + "L";
    request = QString( end_point + "/SelectStructureLocationLinks?ID=" ) + QString::number( id ) + "L";
    download_object.link_list = this->download_json( request, QString( "links-" ) + QString::number( id ) );

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

  const int save_to_file = 0;
  const int load_from_file = 0;

  QList<QVariant> list;

  if ( load_from_file )
  {
    QFile* file = new QFile( "C:\\Users\\amorris\\json-" + file_prefix + ".txt" );

    if ( !file->open( QIODevice::ReadOnly ) )
    {
      std::cerr << "Error opening file for reading\n";
    }

    QTextStream ts( file );
    int num_pages = ts.readLine().toInt();
    for ( int i = 0; i < num_pages; i++ )
    {
      QString qs = ts.readLine();

      QMap<QString, QVariant> map = Json::decode( qs );

      QList<QVariant> this_list = map["value"].toList();
      list.append( this_list );
    }

    file->close();

    return list;
  }

  QList<QString> pages;

  bool more = false;

  do
  {

    QString text = this->download_url( url_string );
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
    QFile* file = new QFile( "C:\\Users\\amorris\\json-" + file_prefix + ".txt" );

    if ( !file->open( QIODevice::WriteOnly ) )
    {
      std::cerr << "Error opening file for writing\n";
    }

    QTextStream ts( file );
    ts << pages.size() << "\n";
    for ( int i = 0; i < pages.size(); i++ )
    {
      ts << pages[i] << "\n";
    }

    file->close();
  }

  return list;
}

//-----------------------------------------------------------------------------
QString Downloader::download_url( QString url_string )
{
  QUrl url = url_string;
  QNetworkRequest request = QNetworkRequest( url );
  request.setRawHeader( "Accept", "application/json" );
  this->reply_ = qnam_.get( request );

  // wait for download
  QEventLoop loop;
  connect( this->reply_, SIGNAL( finished() ), &loop, SLOT( quit() ) );
  loop.exec();

  QString text = this->reply_->readAll();

  return text;
}
