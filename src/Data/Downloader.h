#ifndef VIKING_DATA_DOWNLOADER_H
#define VIKING_DATA_DOWNLOADER_H

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QSharedPointer>
#include <QProgressDialog>
#include <Scale.h>

class Structure;

class DownloadException
{
public:
  DownloadException( QString message )
  {
    this->message_ = message;
  }

  QString message_;
};


class DownloadObject
{
public:
  QList<QVariant> location_list;
  QList<QVariant> link_list;
  QList<QVariant> structure_list;
};

//! Downloads and parses JSON data from viking database
/*!
 * The Downloader downloads and parses JSON data from the viking database
 */
class Downloader : public QObject
{
  Q_OBJECT

public:
  Downloader();
  ~Downloader();

  

  ScaleObject download_scale(QString end_point);

  bool download_cell( QString end_point, int id, DownloadObject &download_object, QProgressDialog &progress );

  QList<QVariant> get_location_list();
  QList<QVariant> get_link_list();

public Q_SLOTS:

private:

  static bool is_valid_odata_response(QMap<QString, QVariant> map);

  static QList<QVariant> load_from_file(QString file_prefix);

  static QList<QVariant> download_json( QString url_string, QString file_prefix );

  static QList<QVariant> download_item( QString request );

  static QString download_url( QString url_string );



  bool http_request_aborted;
};

#endif /* VIKING_DATA_DOWNLOADER_H */
