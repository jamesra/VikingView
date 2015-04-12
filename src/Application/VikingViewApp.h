#ifndef VIKINGVIEW_APPLICATION_VIKINGVIEWAPP_H
#define VIKINGVIEW_APPLICATION_VIKINGVIEWAPP_H

#include <QMainWindow>
#include <QActionGroup>
#include <QSlider>
#include <QLabel>
#include <QTimer>

#include <QFile>
#include <QMap>


#include <Data/Structure.h>
class Viewer;
//class Structure;

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

  virtual void closeEvent( QCloseEvent* event );

public Q_SLOTS:

  void on_action_quit_triggered();
  void on_action_preferences_triggered();

  void on_add_button_clicked();
  void on_delete_button_clicked();

  void on_opacity_slider_valueChanged();
  void on_sampling_slider_valueChanged();

  void on_auto_view_button_clicked();
  void on_cutting_plane_button_clicked();

  void on_connectome_configure_clicked();

  void on_preferences_changed();

private:

  void update_table();


  void import_json( QString json_text );

  /// designer form
  Ui_VikingViewApp* ui_;

  //QList<QSharedPointer<Structure> > structures_;

  //QMap<int, QSharedPointer<Structure> > structures_;

  StructureHash structures_;

  QList< QSharedPointer<Cell> > cells_;

  Viewer* viewer_;
};

#endif /* VIKINGVIEW_APPLICATION_VIKINGVIEWAPP_H */
