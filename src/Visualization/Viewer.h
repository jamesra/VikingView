#ifndef STUDIO_VISUALIZATION_VIEWER_H
#define STUDIO_VISUALIZATION_VIEWER_H

#include <QSharedPointer>

class vtkRenderer;
class vtkLookupTable;
class vtkRenderWindowInteractor;
class vtkImageData;
class vtkCamera;
class vtkGlyph3D;
class vtkSphereSource;
class vtkImageActor;

class DisplayObject;

class Viewer;

class StudioInteractorStyle;

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

  void display_mesh( vtkSmartPointer<vtkPolyData> poly_data );

  void clear_viewer();

  void reset_camera();

private:

  bool visible_;

  vtkSmartPointer<vtkPolyData> poly_data_;

  void update_actors();

  vtkSmartPointer<vtkRenderer>             renderer_;

  vtkSmartPointer<vtkPolyDataMapper>       surface_mapper_;
  vtkSmartPointer<vtkActor>                surface_actor_;

  vtkSmartPointer<vtkLookupTable>          lut_;

  //vtkSmartPointer<StudioInteractorStyle>   style_;

//  vtkSmartPointer<vtkImageActor>           image_actor_;
};

#endif /* STUDIO_VISUALIZATION_VIEWER_H */
