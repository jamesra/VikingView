#ifndef STUDIO_VISUALIZATION_VIEWER_H
#define STUDIO_VISUALIZATION_VIEWER_H

#include <QSharedPointer>
#include <vtkSmartPointer.h>

class vtkRenderer;
class vtkLookupTable;
class vtkRenderWindowInteractor;
class vtkImageData;
class vtkCamera;
class vtkGlyph3D;
class vtkSphereSource;
class vtkImageActor;
class vtkPlane;
class vtkPolyDataMapper;
class vtkActor;
class vtkImplicitPlaneWidget2;
class vtkImplicitPlaneRepresentation;
class vtkIPWCallback;
class vtkOrientationMarkerWidget;

class OrientationController;

class Viewer;
class Structure;

typedef QSharedPointer< Viewer > ViewerHandle;
typedef QVector< ViewerHandle > ViewerList;

//! 3D Viewer
/*!
 * The Viewer class encapsulates all the functionality for visualizing a single DisplayObject
 *
 */
class Viewer
{

public:

  Viewer();
  ~Viewer();

  void set_render_window( vtkRenderWindow* render_window );

  void set_renderer( vtkSmartPointer<vtkRenderer> renderer );
  vtkSmartPointer<vtkRenderer> get_renderer();

  void display_structures( StructureHash structures );

  void display_cells( QList< QSharedPointer<Cell> > cells );

  void clear_viewer();

  void reset_camera();

  void redraw();

  void set_opacity( float opacity );

  void set_clipping_plane( bool clip );

private:

  bool visible_;

  vtkSmartPointer<vtkPolyData> poly_data_;

  void update_actors();

  vtkSmartPointer<vtkRenderer>             renderer_;

  QList<vtkSmartPointer<vtkPolyDataMapper> > surface_mappers_;
  QList<vtkSmartPointer<vtkActor> > surface_actors_;

  vtkSmartPointer<vtkLookupTable>          lut_;

  vtkSmartPointer<vtkImplicitPlaneWidget2> plane_widget_;
  vtkSmartPointer<vtkImplicitPlaneRepresentation> imp_plane_rep_;
  vtkSmartPointer<vtkIPWCallback> callback_;
  vtkSmartPointer<vtkPlane> plane;

  vtkSmartPointer<OrientationController> orientation_controller_;
  vtkSmartPointer<vtkOrientationMarkerWidget> orientation_widget_;
};

#endif /* STUDIO_VISUALIZATION_VIEWER_H */
