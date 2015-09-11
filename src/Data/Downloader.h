#ifndef VIKING_DATA_DOWNLOADER_H
#define VIKING_DATA_DOWNLOADER_H

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QSharedPointer>
#include <QProgressDialog>

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

  bool download_cell( QString end_point, int id, DownloadObject &download_object, QProgressDialog &progress );

  QList<QVariant> get_location_list();
  QList<QVariant> get_link_list();

public Q_SLOTS:

private:

  QList<QVariant> download_json( QString url_string, QString file_prefix );

  QString download_url( QString url_string );

  QNetworkAccessManager qnam_;
  QNetworkReply* reply_;

  bool http_request_aborted;
};

#endif /* VIKING_DATA_DOWNLOADER_H */
