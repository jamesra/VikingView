#include <Data/Downloader.h>
#include <Data/Structure.h>


Downloader::Downloader()
{

}

Downloader::~Downloader()
{

}

//-----------------------------------------------------------------------------
QSharedPointer<Structure> Downloader::download_structure( int id )
{
  QUrl url = QString("http://connectomes.utah.edu/Rabbit/OData/ConnectomeData.svc/Locations/?$filter=ParentID eq ") + QString::number(id);

  QNetworkRequest request = QNetworkRequest( url );
  request.setRawHeader( "Accept", "application/json" );

  this->reply_ = qnam_.get( request );


  // wait for download
  QEventLoop loop;
  connect(this->reply_, SIGNAL(finished()), &loop, SLOT(quit()));
  loop.exec();


  QString location_text = this->reply_->readAll();


  url = QString("http://connectomes.utah.edu/Rabbit/OData/ConnectomeData.svc/SelectStructureLocationLinks?StructureID=") + QString::number(id) + "L";
  request = QNetworkRequest( url );
  request.setRawHeader( "Accept", "application/json" );
  this->reply_ = qnam_.get( request );


  // wait for download
  connect(this->reply_, SIGNAL(finished()), &loop, SLOT(quit()));
  loop.exec();

  QString link_text = this->reply_->readAll();

  std::cerr << "downloaded\n";

  QSharedPointer<Structure> structure = Structure::create_structure(location_text, link_text);

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
{

}

//-----------------------------------------------------------------------------
void Downloader::http_ready_read()
{

}

//-----------------------------------------------------------------------------
void Downloader::update_data_read_progress( qint64 bytes_read, qint64 total_bytes )
{

}
