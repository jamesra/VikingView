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

#include <vtkPolyDataNormals.h>

#include <Visualization/Viewer.h>

//-----------------------------------------------------------------------------
Viewer::Viewer()
{
  this->renderer_ = vtkSmartPointer<vtkRenderer>::New();

  this->surface_actor_ = vtkSmartPointer<vtkActor>::New();
  this->surface_mapper_ = vtkSmartPointer<vtkPolyDataMapper>::New();

  this->visible_ = false;
  /*

     // Create a sphere
     vtkSmartPointer<vtkSphereSource> sphereSource =
     vtkSmartPointer<vtkSphereSource>::New();
     sphereSource->SetCenter(0.0, 0.0, 0.0);
     sphereSource->SetRadius(5.0);

     vtkSmartPointer<vtkPolyDataMapper> mapper = this->surface_mapper_;
     mapper->SetInputConnection(sphereSource->GetOutputPort());

     vtkSmartPointer<vtkActor> actor =
     vtkSmartPointer<vtkActor>::New();
     actor->SetMapper(mapper);
     //vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor =
     //vtkSmartPointer<vtkRenderWindowInteractor>::New();
     //renderWindowInteractor->SetRenderWindow(renderWindow);

     this->renderer_->AddActor(actor);
     this->renderer_->SetBackground(.3, .6, .3); // Background color green
   */
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
void Viewer::update_actors()
{
  if ( !this->visible_ )
  {
    return;
  }

  this->renderer_->RemoveActor( this->surface_actor_ );

  this->renderer_->AddActor( this->surface_actor_ );
}

void Viewer::display_mesh( vtkSmartPointer<vtkPolyData> poly_data )
{

  vtkSmartPointer<vtkPolyDataMapper> mapper = this->surface_mapper_;
  vtkSmartPointer<vtkActor> actor = this->surface_actor_;

  vtkSmartPointer<vtkPolyDataNormals> normals = vtkSmartPointer<vtkPolyDataNormals>::New();
  normals->SetInputData( poly_data );

/*
   #if VTK_MAJOR_VERSION <= 5
   mapper->SetInput( poly_data );
   #else
   mapper->SetInputData( poly_data );
   #endif
   //
 */

  mapper->SetInputConnection( normals->GetOutputPort() );

  actor->SetMapper( mapper );
  actor->GetProperty()->SetDiffuseColor( 1, 191.0 / 255.0, 0 );
  actor->GetProperty()->SetSpecular( 0.2 );
  actor->GetProperty()->SetSpecularPower( 15 );
  mapper->ScalarVisibilityOff();

  this->renderer_->AddActor( this->surface_actor_ );
  //this->update_actors();
  this->renderer_->SetBackground( .3, .6, .3 ); // Background color green
}

void Viewer::set_render_window( vtkRenderWindow* render_window )
{
  render_window->AddRenderer( this->renderer_ );
}
