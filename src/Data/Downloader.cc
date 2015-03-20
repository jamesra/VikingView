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

  QList<QVariant> location_list;
  QList<QVariant> link_list;
  QList<QVariant> structure_list;

  QString end_point = "http://connectomes.utah.edu/Rabbit/OData/ConnectomeData.svc";
  //QString end_point = "http://websvc1.connectomes.utah.edu/RC1/OData/ConnectomeData.svc";

  QString request = QString( end_point + "/Structures/?$filter=ParentID eq " )
                    + QString::number( id ) + " or ID eq " + QString::number( id );
  structure_list = this->download_json( request, QString( "structures-" ) + QString::number( id ) );

  request = QString( end_point + "/Locations/?$filter=ParentID eq " )
            + QString::number( id );
  location_list = this->download_json( request, QString( "locations-" ) + QString::number( id ) );

  request = QString( end_point + "/SelectStructureLocationLinks?StructureID=" )
            + QString::number( id ) + "L";

  link_list = this->download_json( request, QString( "links-" ) + QString::number( id ) );
  std::cerr << "downloaded\n";

  structure = Structure::create_structure( id, structure_list, location_list, link_list );

  return structure;
}

//-----------------------------------------------------------------------------
QList<QVariant> Downloader::download_json( QString url_string, QString file_prefix )
{

  const int save_to_file = 1;
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

    list.append( map["value"].toList() );

    more = map.contains( "odata.nextLink" );

    if ( more )
    {
      QString link = map["odata.nextLink"].asString();
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
