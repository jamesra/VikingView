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

#include <Data/Structure.h>

#include <Visualization/Viewer.h>

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
    actor->GetProperty()->SetDiffuseColor( 1, 191.0 / 255.0, 0 );
    actor->GetProperty()->SetSpecular( 0.2 );
    actor->GetProperty()->SetSpecularPower( 15 );
    mapper->ScalarVisibilityOff();

    this->renderer_->AddActor( actor );

    //this->update_actors();
    //this->renderer_->SetBackground( .3, .6, .3 ); // Background color green
  }

  this->renderer_->ResetCamera();
  this->renderer_->Render();
  this->renderer_->GetRenderWindow()->Render();
}
