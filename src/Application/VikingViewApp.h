#ifndef VIKINGVIEW_APPLICATION_VIKINGVIEWAPP_H
#define VIKINGVIEW_APPLICATION_VIKINGVIEWAPP_H

#include <QMainWindow>
#include <QActionGroup>
#include <QSlider>
#include <QLabel>
#include <QTimer>

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>

class Viewer;


// Forward Qt class declarations
class Ui_VikingViewApp;


//! Main VikingView window
/*!
 * This class represents the primary VikingView window interface
 */
class VikingViewApp : public QMainWindow
{
  Q_OBJECT
public:

  VikingViewApp( int argc, char** argv );
  ~VikingViewApp();

  //virtual void closeEvent( QCloseEvent* event );

public Q_SLOTS:

  void on_add_button_clicked();
  void on_delete_button_clicked();


  void httpFinished();
  void httpReadyRead();
  void updateDataReadProgress(qint64 bytesRead, qint64 totalBytes);




private:


  void load_structure( int id );

  void import_json(QString json_text);




  /// designer form
  Ui_VikingViewApp* ui_;

  QFile *file;
  QNetworkAccessManager qnam;
  QNetworkReply *reply;

  bool httpRequestAborted;


  Viewer *viewer_;

};

#endif /* VIKINGVIEW_APPLICATION_VIKINGVIEWAPP_H */
