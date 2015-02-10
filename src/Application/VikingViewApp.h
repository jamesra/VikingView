#ifndef VIKINGVIEW_APPLICATION_VIKINGVIEWAPP_H
#define VIKINGVIEW_APPLICATION_VIKINGVIEWAPP_H

#include <QMainWindow>
#include <QActionGroup>
#include <QSlider>
#include <QLabel>
#include <QTimer>

#include <QFile>

class Viewer;
class Structure;

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

  void initialize_vtk();

  void load_structure( int id );

  //virtual void closeEvent( QCloseEvent* event );

public Q_SLOTS:

  void on_action_quit_triggered();

  void on_add_button_clicked();
  void on_delete_button_clicked();

  void on_opacity_slider_valueChanged();
  void on_sampling_slider_valueChanged();

  void on_auto_view_button_clicked();
  void on_cutting_plane_button_clicked();

private:

  void update_table();


  void import_json( QString json_text );

  /// designer form
  Ui_VikingViewApp* ui_;

  QList<QSharedPointer<Structure> > structures_;

  Viewer* viewer_;
};

#endif /* VIKINGVIEW_APPLICATION_VIKINGVIEWAPP_H */
