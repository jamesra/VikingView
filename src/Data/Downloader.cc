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
QSharedPointer<Structure> Downloader::download_structure( int id )
{
  QSharedPointer<Structure> structure;

  const int save_to_file = 0;
  const int load_from_file = 0;

  QList<QVariant> location_list;
  QList<QVariant> link_list;

  // download locations
  if ( !load_from_file )
  {
    QString request = QString( "http://connectomes.utah.edu/Rabbit/OData/ConnectomeData.svc/Locations/?$filter=ParentID eq " ) + QString::number( id );
    location_list = this->download_json( request, QString( "locations-" ) + QString::number( id ) );

    request = QString( "http://connectomes.utah.edu/Rabbit/OData/ConnectomeData.svc/SelectStructureLocationLinks?StructureID=" ) + QString::number( id ) + "L";

    link_list = this->download_json( request, QString( "links-" ) + QString::number( id ) );
    std::cerr << "downloaded\n";

    if ( save_to_file )
    {
/*
      QFile* file = new QFile( "C:\\Users\\amorris\\json-locations-" + QString::number( id ) + ".txt" );

      if ( !file->open( QIODevice::WriteOnly ) )
      {
        std::cerr << "Error opening file for writing\n";
        return structure;
      }

      QTextStream ts( file );
      ts << location_list.size() << "\n";
      for ( int i = 0; i < location_list.size(); i++ )
       {
        ts << location_list[i].asString() << "\n";
      }

      file->close();
 */

/*
      file = new QFile( "C:\\Users\\amorris\\json-links-" + QString::number( id ) + ".txt" );

      if ( !file->open( QIODevice::WriteOnly ) )
      {
        std::cerr << "Error opening file for writing\n";
        return structure;
      }

      QTextStream ts2( file );
      ts2 << link_list.size() << "\n";
      for ( int i = 0; i < link_list.size(); i++ )
      {
        ts2 << link_list[i].asString() << "\n";
      }
      file->close();
 */
    }
  }
  else
  {

    QFile* file = new QFile( "C:\\Users\\amorris\\json-locations-" + QString::number( id ) + ".txt" );

    if ( !file->open( QIODevice::ReadOnly ) )
    {
      return structure;
    }

    int num_items;
    QTextStream ts( file );
    ts >> num_items;
    for ( int i = 0; i < num_items; i++ )
    {
      QString qs;
      ts >> qs;
      location_list.append( qs );
    }

    file = new QFile( "C:\\Users\\amorris\\json-links-" + QString::number( id ) + ".txt" );

    if ( !file->open( QIODevice::ReadOnly ) )
    {
      return structure;
    }

    QTextStream ts2( file );
    ts2 >> num_items;
    for ( int i = 0; i < num_items; i++ )
    {
      QString qs;
      ts2 >> qs;
      link_list.append( qs );
    }
  }

  structure = Structure::create_structure( id, location_list, link_list );

/*
   //this->file = new QFile( "C:\\Users\\amorris\\json.txt" );

   this->http_request_aborted = false;

   connect( this->reply_, SIGNAL( finished() ),
    this, SLOT( http_finished() ) );
   connect( this->reply_, SIGNAL( readyRead() ),
    this, SLOT( http_ready_read() ) );
   connect( this->reply_, SIGNAL( downloadProgress( qint64, qint64 ) ),
    this, SLOT( update_data_read_progress( qint64, qint64 ) ) );
 */

  return structure;
}

//-----------------------------------------------------------------------------
void Downloader::http_finished()
{}

//-----------------------------------------------------------------------------
void Downloader::http_ready_read()
{}

//-----------------------------------------------------------------------------
void Downloader::update_data_read_progress( qint64 bytes_read, qint64 total_bytes )
{}

//-----------------------------------------------------------------------------
QList<QVariant> Downloader::download_json( QString url_string, QString file_prefix )
{

  const int save_to_file = 1;
  const int load_from_file = 1;

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

    list.append( map["value"].toList() );

    more = map.contains( "odata.nextLink" );

    if ( more )
    {
      QString link = map["odata.nextLink"].asString();
      QString next_page = url_string.split( ".svc" )[0] + ".svc" + link.split( ".svc" )[1];
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
