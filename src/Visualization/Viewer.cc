#include <vtkPointData.h>
#include <vtkUnsignedLongArray.h>
#include <vtkSphereSource.h>
#include <vtkGlyph3D.h>
#include <vtkProperty.h>
#include <vtkLookupTable.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkTransform.h>
#include <vtkRenderer.h>
#include <vtkImageActor.h>
#include <vtkImageData.h>
#include <vtkRenderWindow.h>
#include <vtkProperty.h>
#include <vtkTextActor.h>
#include <vtkTextProperty.h>
#include <vtkCornerAnnotation.h>
#include <vtkPointPicker.h>
#include <vtkIdTypeArray.h>
#include <vtkPropPicker.h>
#include <vtkCellPicker.h>
#include <vtkCell.h>
#include <vtkPolyDataMapper.h>

#include <vtkPolyDataNormals.h>
#include <vtkImplicitPlaneRepresentation.h>
#include <vtkImplicitPlaneWidget2.h>
#include <vtkCommand.h>
#include <vtkPlane.h>

#include <Data/Structure.h>

#include <Visualization/Viewer.h>

// Callback for the interaction
// This does the actual work: updates the vtkPlane implicit function.
// This in turn causes the pipeline to update and clip the object.
class vtkIPWCallback : public vtkCommand
{
public:
  static vtkIPWCallback* New()
  { return new vtkIPWCallback; }
  virtual void Execute( vtkObject* caller, unsigned long, void* )
  {
    vtkImplicitPlaneWidget2* plane_widget =
      reinterpret_cast<vtkImplicitPlaneWidget2*>( caller );
    vtkImplicitPlaneRepresentation* rep =
      reinterpret_cast<vtkImplicitPlaneRepresentation*>( plane_widget->GetRepresentation() );
    rep->GetPlane( this->Plane );
  }
  vtkIPWCallback() : Plane( 0 ), Actor( 0 ) {}
  vtkPlane* Plane;
  vtkActor* Actor;
};

//-----------------------------------------------------------------------------
Viewer::Viewer()
{
  this->renderer_ = vtkSmartPointer<vtkRenderer>::New();

  this->visible_ = false;
}

//-----------------------------------------------------------------------------
Viewer::~Viewer()
{}

//-----------------------------------------------------------------------------
void Viewer::clear_viewer()
{
  this->renderer_->RemoveAllViewProps();
  this->visible_ = false;
}

//-----------------------------------------------------------------------------
void Viewer::reset_camera()
{
  this->renderer_->ResetCamera();
}

//-----------------------------------------------------------------------------
void Viewer::set_renderer( vtkSmartPointer<vtkRenderer> renderer )
{
  this->renderer_ = renderer;
}

//-----------------------------------------------------------------------------
vtkSmartPointer<vtkRenderer> Viewer::get_renderer()
{
  return this->renderer_;
}

//-----------------------------------------------------------------------------
void Viewer::set_render_window( vtkRenderWindow* render_window )
{
  render_window->AddRenderer( this->renderer_ );
}

//-----------------------------------------------------------------------------
void Viewer::display_structures( QList<QSharedPointer<Structure> > structures )
{
  this->surface_actors_.clear();
  this->surface_mappers_.clear();
  this->renderer_->RemoveAllViewProps();

  foreach( QSharedPointer<Structure> s, structures ) {
    vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();

    vtkSmartPointer<vtkPolyData> mesh = s->get_mesh();

    vtkSmartPointer<vtkPolyDataNormals> normals = vtkSmartPointer<vtkPolyDataNormals>::New();
    normals->SetInputData( mesh );

    mapper->SetInputConnection( normals->GetOutputPort() );

    actor->SetMapper( mapper );

    QColor color = s->get_color();

    //actor->GetProperty()->SetDiffuseColor( 1, 191.0 / 255.0, 0 );
    actor->GetProperty()->SetDiffuseColor( color.red() / 255.0, color.green() / 255.0, color.blue() / 255.0 );
    actor->GetProperty()->SetSpecular( 0.2 );
    actor->GetProperty()->SetSpecularPower( 15 );
    actor->GetProperty()->BackfaceCullingOn();

    mapper->ScalarVisibilityOff();

    this->renderer_->AddActor( actor );

    this->surface_actors_.append( actor );
    this->surface_mappers_.append( mapper );
  }

  this->renderer_->ResetCamera();
  this->renderer_->Render();
  this->renderer_->GetRenderWindow()->Render();
}

//-----------------------------------------------------------------------------
void Viewer::set_opacity( float opacity )
{
  foreach( vtkSmartPointer<vtkActor> actor, this->surface_actors_ ) {
    actor->GetProperty()->SetOpacity( opacity );
  }
  this->renderer_->GetRenderWindow()->Render();
}

//-----------------------------------------------------------------------------
void Viewer::redraw()
{
  this->renderer_->Render();
  this->renderer_->GetRenderWindow()->Render();
}

//-----------------------------------------------------------------------------
void Viewer::set_clipping_plane( bool clip )
{
  if ( !clip )
  {

    for ( int i = 0; i < this->surface_mappers_.size(); i++ )
    {
      this->surface_mappers_[i]->RemoveAllClippingPlanes();
    }

    if ( this->plane_widget_ )
    {
      this->plane_widget_->Off();
    }
  }
  else
  {

    plane = vtkSmartPointer<vtkPlane>::New();

    callback_ = vtkSmartPointer<vtkIPWCallback>::New();
    callback_->Plane = plane;

    imp_plane_rep_ = vtkSmartPointer<vtkImplicitPlaneRepresentation>::New();
    imp_plane_rep_->SetPlaceFactor( 1.25 ); // This must be set prior to placing the widget

    double bounds[6];
    this->renderer_->ComputeVisiblePropBounds( bounds );

    double center[3];
    center[0] = ( bounds[0] + bounds[1] ) / 2.0;
    center[1] = ( bounds[2] + bounds[3] ) / 2.0;
    center[2] = ( bounds[4] + bounds[5] ) / 2.0;

    imp_plane_rep_->PlaceWidget( bounds );
    imp_plane_rep_->SetNormal( plane->GetNormal() );
    imp_plane_rep_->SetOrigin( center );
    imp_plane_rep_->GetPlane( this->plane );

    for ( int i = 0; i < this->surface_mappers_.size(); i++ )
    {
      this->surface_mappers_[i]->AddClippingPlane( plane );
    }

    plane_widget_ = vtkSmartPointer<vtkImplicitPlaneWidget2>::New();
    plane_widget_->SetInteractor( this->renderer_->GetRenderWindow()->GetInteractor() );
    plane_widget_->SetRepresentation( imp_plane_rep_ );
    plane_widget_->AddObserver( vtkCommand::InteractionEvent, callback_ );
    plane_widget_->On();
  }

  this->redraw();
}
