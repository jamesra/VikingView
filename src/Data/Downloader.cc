#include <Data/Downloader.h>
#include <Data/Structure.h>

#include <QString>

Downloader::Downloader()
{}

Downloader::~Downloader()
{}

//-----------------------------------------------------------------------------
QSharedPointer<Structure> Downloader::download_structure( int id )
{
  QSharedPointer<Structure> structure;

  const int load_from_file = 1;
  const int save_to_file = 0;

  QString location_text;
  QString link_text;

  // download locations
  if ( !load_from_file )
  {
    QUrl url = QString( "http://connectomes.utah.edu/Rabbit/OData/ConnectomeData.svc/Locations/?$filter=ParentID eq " ) + QString::number( id );
    QNetworkRequest request = QNetworkRequest( url );
    request.setRawHeader( "Accept", "application/json" );
    this->reply_ = qnam_.get( request );

    // wait for download
    QEventLoop loop;
    connect( this->reply_, SIGNAL( finished() ), &loop, SLOT( quit() ) );
    loop.exec();

    location_text = this->reply_->readAll();

    url = QString( "http://connectomes.utah.edu/Rabbit/OData/ConnectomeData.svc/SelectStructureLocationLinks?StructureID=" ) + QString::number( id ) + "L";
    request = QNetworkRequest( url );
    request.setRawHeader( "Accept", "application/json" );
    this->reply_ = qnam_.get( request );

    // wait for download
    connect( this->reply_, SIGNAL( finished() ), &loop, SLOT( quit() ) );
    loop.exec();

    link_text = this->reply_->readAll();
    std::cerr << "downloaded\n";


    if (save_to_file)
    {
      QFile *file = new QFile( "C:\\Users\\amorris\\json-locations-" + QString::number(id) + ".txt" );

      if ( !file->open( QIODevice::WriteOnly ) )
      {
        return structure;
      }

      QTextStream ts(file);
      ts << location_text;
      file->close();

      file = new QFile( "C:\\Users\\amorris\\json-links-" + QString::number(id) + ".txt" );

      if ( !file->open( QIODevice::WriteOnly ) )
      {
        return structure;
      }

      QTextStream ts2(file);
      ts2 << link_text;
      //file->write(link_text);
      file->close();

    }

  }
  else
  {


    QFile *file = new QFile( "C:\\Users\\amorris\\json-locations-" + QString::number(id) + ".txt" );

    if ( !file->open( QIODevice::ReadOnly ) )
    {
      return structure;
    }

    location_text = file->readAll();

    file = new QFile( "C:\\Users\\amorris\\json-links-" + QString::number(id) + ".txt" );

    if ( !file->open( QIODevice::ReadOnly ) )
    {
      return structure;
    }

    link_text = file->readAll();


}

  structure = Structure::create_structure( id, location_text, link_text );

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
