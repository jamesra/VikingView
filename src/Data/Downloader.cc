#include <Data/Downloader.h>
#include <Data/Structure.h>
#include <Data/Json.h>

#include <QString>
#include <QEventLoop>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>

Downloader::Downloader()
{}

Downloader::~Downloader()
{}

//-----------------------------------------------------------------------------
bool Downloader::download_cell( QString end_point, int id, DownloadObject &download_object, QProgressDialog &progress )
{
  try{

    // http://websvc1.connectomes.utah.edu/RC1/OData/Structures(180)/Children

    //QString request = QString( end_point + "/Structures/?$filter=ParentID eq " )
    // + QString::number( id ) + " or ID eq " + QString::number( id );

    //http://websvc1.connectomes.utah.edu/RC1/OData/Structures?$filter=(ID eq 180 or ParentID eq 180)

    QString request = QString( end_point + "/Structures?$filter=(ID eq " ) + QString::number( id )
                      + " or ParentID eq " + QString::number( id ) + ")&$select=ID,TypeID";
    download_object.structure_list = this->download_json( request, QString( "structures-" ) + QString::number( id ) );

    std::cerr << "structure list length = " << download_object.structure_list.size() << "\n";
    int idx = 0;

    progress.setMaximum( download_object.structure_list.size() );

    foreach( QVariant var, download_object.structure_list ) {
      QMap<QString, QVariant> item = var.toMap();
      int id = item["ID"].toLongLong();

      request = QString( end_point + "/Structures(" ) + QString::number( id ) + ")/Locations?$select=ID,VolumeX,VolumeY,Z,Radius,ParentID";
      QList<QVariant> list = this->download_json( request, QString( "locations-" ) + QString::number( id ) + "-" + QString::number( idx ) );
      download_object.location_list.append( list );

      request = QString( end_point + "/Structures(" ) + QString::number( id ) + ")/LocationLinks?$select=A,B";
      list = this->download_json( request, QString( "links-" ) + QString::number( id ) + "-" + QString::number( idx ) );
      download_object.link_list.append( list );

      std::cerr << idx << "/" << download_object.structure_list.size() << "\n";
      idx++;

      progress.setValue(idx);
    }

    //request = QString( end_point + "/SelectStructureLocations?ID=" ) + QString::number( id ) + "L" + "&$format=json";
    //request = QString( end_point + "/Structures(" ) + QString::number( id ) + ")/Locations";
    //download_object.location_list = this->download_json( request, QString( "locations-" ) + QString::number( id ) );

    //request = QString( end_point + "/SelectStructureLocationLinks?StructureID=" ) + QString::number( id ) + "L";
    //request = QString( end_point + "/SelectStructureLocationLinks?ID=" ) + QString::number( id ) + "L";
    //request = QString( end_point + "/Structures(" ) + QString::number( id ) + ")/LocationLinks";
    //download_object.link_list = this->download_json( request, QString( "links-" ) + QString::number( id ) );

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
