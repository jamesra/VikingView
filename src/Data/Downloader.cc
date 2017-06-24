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
#include <QJsonArray>
#include <QJsonObject>

Downloader::Downloader()
{}

Downloader::~Downloader()
{}

bool Downloader::is_valid_odata_response(QMap<QString, QVariant> map)
{
	return map.contains("@odata.context");
}

QList<long> Downloader::FetchStructureIDsFromODataQuery(QString end_point, QString query)
{
	QString url_string = QString(end_point + "/" + query);
	QString response = Downloader::download_url(url_string);
	QMap<QString, QVariant> map = Json::decode(response);
	QList<long> IDs;

	if (!is_valid_odata_response(map))
	{
		QString message = "Error downloading url: " + url_string + "\n\n" + response.left(256);
		std::cout << message.toStdString();
		return IDs;
	}

	//OK, the response could be an array of integers, or a set of objects with an ID field.
	if (map.contains("value"))
	{
		if (!map.value("value").canConvert<QVariantList>())
			return IDs;

		QVariantList items = map.value("value").toList();
		foreach(QVariant item, items)
		{
			if (item.canConvert<int>())
			{
				IDs.append(item.toLongLong());
			}
			else if (item.canConvert<QMap<QString, QVariant>>())
			{
				QMap<QString, QVariant> obj = item.toMap();
				if (obj.contains("ID"))
				{
					IDs.append(obj["ID"].toInt());
				}
			}
		}
	}

	return IDs; 
}

QSharedPointer<ScaleObject> Downloader::download_scale(QString end_point)
{
	QString url_string = QString(end_point + "/Scale");
	QString response = Downloader::download_url(url_string);
	QMap<QString, QVariant> map = Json::decode(response);

	if (!is_valid_odata_response(map))
	{
		QString message = "Error downloading url: " + url_string + "\n\n" + response.left(256);
		throw DownloadException(message);
	}

	QMap<QString, QVariant> X = map["X"].toMap();
	QMap<QString, QVariant> Y = map["Y"].toMap();
	QMap<QString, QVariant> Z = map["Z"].toMap();
	
	AxisScale Xscaleobj = AxisScale(X["Units"].toString(), X["Value"].toDouble());
	AxisScale Yscaleobj = AxisScale(Y["Units"].toString(), Y["Value"].toDouble());
	AxisScale Zscaleobj = AxisScale(Z["Units"].toString(), Z["Value"].toDouble());

	QSharedPointer<ScaleObject> scale = QSharedPointer<ScaleObject>(new ScaleObject(Xscaleobj, Yscaleobj, Zscaleobj));
	return scale;
}

QList<QVariant> Downloader::download_structures(QString end_point, QList<long> StructureIDs)
{
	QList<QString> requests;

	foreach(long StructureID, StructureIDs)
	{
		QString request = QString(end_point + "/Structures?$filter=(ID eq ") + QString::number(StructureID)
			+ " or ParentID eq " + QString::number(StructureID) + ")&$select=ID,TypeID,ParentID,Label";

		requests.append(request);
	}

	QList< QList<QVariant> > downloaded = QtConcurrent::blockingMapped(requests, download_item);;
	
	QList<QVariant> rows;
	foreach(QList<QVariant> row, downloaded)
	{
		rows.append(row);
	}

	return rows;
}

QList<QVariant> Downloader::download_locations(QString end_point, QList<long> StructureIDs)
{
	QList<QString> requests;

	foreach(long StructureID, StructureIDs)
	{
		QString request = QString(end_point + "/Structures(") + QString::number(StructureID) + ")/Locations?$select=ID,VolumeX,VolumeY,Z,Radius,ParentID,VolumeShape";
		requests.append(request);
	}

	QList< QList<QVariant> > downloaded = QtConcurrent::blockingMapped(requests, download_item);

	QList<QVariant> rows;
	foreach(QList<QVariant> row, downloaded)
	{
		rows.append(row);
	}

	return rows;
}

QList<QVariant> Downloader::download_locationlinks(QString end_point, QList<long> StructureIDs)
{
	QList<QString> requests;

	foreach(long StructureID, StructureIDs)
	{
		QString request = QString(end_point + "/Structures(") + QString::number(StructureID) + ")/LocationLinks?$select=A,B";
		requests.append(request);
	}

	QList< QList<QVariant> > downloaded = QtConcurrent::blockingMapped(requests, download_item);

	QList<QVariant> rows;
	foreach(QList<QVariant> row, downloaded)
	{
		rows.append(row);
	}

	return rows;
}

//-----------------------------------------------------------------------------
bool Downloader::download_cells( QString end_point, QList<long> ids, DownloadObject &download_object, ProgressReporter &progress )
{
  try {

	progress.set_min(0);
	progress.set_max(3);

	QElapsedTimer timer;
	timer.start();

    // set number of threads to download and parse
	int idealDownloadThreadCount = QThread::idealThreadCount() * 2;

	if(QThreadPool::globalInstance()->maxThreadCount() < idealDownloadThreadCount)
		QThreadPool::globalInstance()->setMaxThreadCount(idealDownloadThreadCount);

	QString message = "Downloading " + QString::number(ids.count()) + " structures";
	progress(0, message);

	download_object.structure_list = download_structures(end_point, ids);

    std::cerr << "structure list length = " << download_object.structure_list.size() << "\n";

    if (download_object.structure_list.empty())
    {
	  std::cerr << "error: tried to load cell id with no structures" << "\n";
	  QMessageBox::critical(0, "Error loading cell", "There are no structures associated with the requested cell ID. Are you sure you entered the cell ID correctly?");
	  return false;
	}

	QList<long> AllStructureIDs;
	foreach(QVariant struct_json, download_object.structure_list)
	{
		QMap<QString, QVariant> item = struct_json.toMap();
		int id = item["ID"].toLongLong();
		AllStructureIDs.append(id);
	}

	message = "Downloading " + QString::number(AllStructureIDs.count()) + " structure annotations";
	progress(1, message);
	
	download_object.location_list = download_locations(end_point, AllStructureIDs);

	progress(2, "Downloading annotation links");

	download_object.link_list = download_locationlinks(end_point, AllStructureIDs);
	 
	progress(3, "Download completed");

    std::cerr << "Download took: " << timer.elapsed() / 1000.0 << " seconds\n";

	QThreadPool::globalInstance()->setMaxThreadCount(QThread::idealThreadCount());
    return true;
  }
  catch ( DownloadException e )
  {
    std::cerr << e.message_.toStdString() << "\n";
    QMessageBox::critical( 0, "Error", e.message_ );
  } 

  QThreadPool::globalInstance()->setMaxThreadCount(QThread::idealThreadCount());
  return false;
}

QList<QVariant> Downloader::load_from_file(QString file_prefix)
{
	QList<QVariant> list;

	QFile* file = new QFile("/tmp/json-" + file_prefix + ".txt");

	if (!file->open(QIODevice::ReadOnly))
	{
		std::cerr << "Error opening file for reading\n";
	}

	QTextStream ts(file);

	QMap<QString, QVariant> map = Json::decode(ts.readAll());

	QList<QVariant> this_list = map["value"].toList();
	list.append(this_list);

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

//-----------------------------------------------------------------------------
QList<QVariant> Downloader::download_json( QString url_string, QString file_prefix )
{
  //std::cerr << "download_json(" << url_string.toStdString() << "\n";
  const int save_to_file = 0;
  const int load_from_file = 0;

  QList<QVariant> list;

  if ( load_from_file )
  {
	  return Downloader::load_from_file(file_prefix);
  }

  QList<QString> pages;

  bool more = false;

  do
  {

    QString text = Downloader::download_url( url_string );
    pages.append( text );

    QMap<QString, QVariant> map = Json::decode( text );

    if ( !is_valid_odata_response(map))
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
