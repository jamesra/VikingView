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
#include <vtkAxesActor.h>
#include <vtkPolyDataNormals.h>
#include <vtkImplicitPlaneRepresentation.h>
#include <vtkImplicitPlaneWidget2.h>
#include <vtkCommand.h>
#include <vtkPlane.h>
#include <vtkOrientationMarkerWidget.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkCamera.h>

#include <QKeyEvent>

#include <Data/Structure.h>
#include <Application/Preferences.h>

#include <Visualization/Viewer.h>

// Callback for the interaction
// This does the actual work: updates the vtkPlane implicit function.
// This in turn causes the pipeline to update and clip the object.
class vtkIPWCallback : public vtkCommand
{
public:
	static vtkIPWCallback* New()
	{
		return new vtkIPWCallback;
	}
	virtual void Execute(vtkObject* caller, unsigned long, void*)
	{
		vtkImplicitPlaneWidget2* plane_widget =
			reinterpret_cast<vtkImplicitPlaneWidget2*>(caller);
		vtkImplicitPlaneRepresentation* rep =
			reinterpret_cast<vtkImplicitPlaneRepresentation*>(plane_widget->GetRepresentation());
		rep->GetPlane(this->Plane);
	}
	vtkIPWCallback() : Plane(0), Actor(0) {}
	vtkPlane* Plane;
	vtkActor* Actor;
};

void SetCameraToXY_ZIn(vtkCamera* camera)
{
	camera->SetPosition(0, 0, 1);
	camera->SetViewUp(0, 1, 0);
}

void SetCameraToXY_ZOut(vtkCamera* camera)
{
	camera->SetPosition(0, 0, -1);
	camera->SetViewUp(0, -1, 0);
}

void SetCameraToYZ_XIn(vtkCamera* camera)
{
	camera->SetPosition(1, 0, 0);
	camera->SetViewUp(0, 0, 1);
}

void SetCameraToYZ_XOut(vtkCamera* camera)
{
	camera->SetPosition(-1, 0, 0);
	camera->SetViewUp(0, 0, 1);
}

void SetCameraToZX_YIn(vtkCamera* camera)
{
	camera->SetPosition(0, 1, 0);
	camera->SetViewUp(0, 0, 1);
}

void SetCameraToZX_YOut(vtkCamera* camera)
{
	camera->SetPosition(0, -1, 0);
	camera->SetViewUp(0, 0, 1);
}

//-----------------------------------------------------------------------------
class OrientationController : public vtkCommand
{
public:
	static OrientationController* New()
	{
		return new OrientationController;
	}

	OrientationController()
	{}

	void Execute(vtkObject* obj, unsigned long event_id, void* call_data)
	{
		// get QKeyEvent
		QKeyEvent* key_event = static_cast<QKeyEvent*>(call_data);

		if (event_id == vtkCommand::KeyPressEvent)
		{
			vtkCamera* camera = this->viewer_->get_renderer()->GetActiveCamera();
			Qt::Key keyPressed = (Qt::Key)key_event->key();
			bool ShiftPressed = key_event->modifiers() & Qt::ShiftModifier;
			float distance = camera->GetDistance();
			float parallel_scale = camera->GetParallelScale();
			bool found = true;

			switch (keyPressed)
			{
			case Qt::Key_X:
				if (ShiftPressed)
					SetCameraToZX_YIn(camera);
				else
					SetCameraToZX_YOut(camera);
				break;
			case Qt::Key_Y:
				if (ShiftPressed)
					SetCameraToYZ_XIn(camera);
				else
					SetCameraToYZ_XOut(camera);
				break;
			case Qt::Key_Z:
				if (ShiftPressed)
					SetCameraToXY_ZIn(camera);
				else
					SetCameraToXY_ZOut(camera);
				break;
			case Qt::Key_A:
				if (ShiftPressed)
					SetCameraToZX_YOut(camera);
					break;
			case Qt::Key_P:
				if (ShiftPressed)
					SetCameraToZX_YIn(camera);
					break;
			case Qt::Key_R:
				if (ShiftPressed)
					SetCameraToYZ_XOut(camera);
					break;
			case Qt::Key_L:
				if (ShiftPressed)
					SetCameraToYZ_XIn(camera);
					break;
			case Qt::Key_S:
				if (ShiftPressed)
					SetCameraToXY_ZIn(camera);
					break;
			case Qt::Key_I:
				if (ShiftPressed)
					SetCameraToXY_ZOut(camera);
					break; 
			default:
				found = false;
			}

			if (found)
			{
				camera->SetFocalPoint(0, 0, 0);
				camera->ComputeViewPlaneNormal();
				camera->OrthogonalizeViewUp();

				this->viewer_->get_renderer()->ResetCamera();

				double vn[3], center[3];
				camera->GetFocalPoint(center);
				camera->GetViewPlaneNormal(vn);
				camera->SetPosition(center[0] + distance * vn[0], center[1] + distance * vn[1], center[2] + distance * vn[2]);
				camera->SetParallelScale(parallel_scale);

				this->viewer_->get_renderer()->ResetCameraClippingRange();
				this->viewer_->redraw();
			}
		}
	}

	Viewer* viewer_;
};

//-----------------------------------------------------------------------------
Viewer::Viewer(QSharedPointer< CommandLineArgs > command_line_args)
	: command_line_args_(command_line_args)
{
	this->renderer_ = vtkSmartPointer<vtkRenderer>::New();

	this->visible_ = false;

	this->orientation_controller_ = vtkSmartPointer<OrientationController>::New();
	this->orientation_controller_->viewer_ = this;
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
void Viewer::set_renderer(vtkSmartPointer<vtkRenderer> renderer)
{
	this->renderer_ = renderer;
}

//-----------------------------------------------------------------------------
vtkSmartPointer<vtkRenderer> Viewer::get_renderer()
{
	return this->renderer_;
}

//-----------------------------------------------------------------------------
void Viewer::set_render_window(vtkRenderWindow* render_window)
{
	render_window->AddRenderer(this->renderer_);

	vtkSmartPointer<vtkAxesActor> axes =
		vtkSmartPointer<vtkAxesActor>::New();

	// Don't add the orientation widget if an OBJ export is being run
	if (!command_line_args_->command_used("export")
		|| !command_line_args_->command_parameters("export").contains("obj"))
	{
		this->orientation_widget_ = vtkSmartPointer<vtkOrientationMarkerWidget>::New();
		this->orientation_widget_->SetOutlineColor(0.9300, 0.5700, 0.1300);
		this->orientation_widget_->SetOrientationMarker(axes);
		this->orientation_widget_->SetInteractor(render_window->GetInteractor());
		this->orientation_widget_->SetViewport(0.80, 0.80, 1, 1);
		this->orientation_widget_->SetEnabled(1);
		//this->orientation_widget_->InteractiveOn();
	}

	render_window->GetInteractor()->AddObserver(vtkCommand::KeyPressEvent, this->orientation_controller_);
}

void Viewer::add_structure_to_view(QSharedPointer<Structure> s)
{
	vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
	vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();

	vtkSmartPointer<vtkPolyData> mesh = s->get_mesh_tubes();

	if (mesh)
	{

		mesh = this->scale_mesh(s);

		mapper->SetInputData(mesh);
		actor->SetMapper(mapper);

		QColor color = s->get_color();

		actor->GetProperty()->SetDiffuseColor(color.red() / 255.0, color.green() / 255.0, color.blue() / 255.0);
		actor->GetProperty()->SetOpacity(color.alpha() / 255.0);
		actor->GetProperty()->SetSpecular(0.2);
		actor->GetProperty()->SetSpecularPower(15);
		actor->GetProperty()->BackfaceCullingOn();

		//actor->GetProperty()->SetRepresentationToWireframe();

		mapper->ScalarVisibilityOff();
		//mapper->ScalarVisibilityOn();

		this->renderer_->AddActor(actor);

		this->surface_actors_.append(actor);
		this->surface_mappers_.append(mapper);
	}

	foreach(QSharedPointer<Structure> child, s->structures.values()) {
		add_structure_to_view(child);
	}

}

//-----------------------------------------------------------------------------
void Viewer::display_cells(QList< QSharedPointer<Structure> > cells, bool reset_camera)
{
	if (!this->renderer_)
	{
		return;
	}

	this->surface_actors_.clear();
	this->surface_mappers_.clear();
	this->renderer_->RemoveAllViewProps();

	foreach(QSharedPointer<Structure> cell, cells) {
		add_structure_to_view(cell); 
	}

	if (reset_camera)
	{
		this->renderer_->ResetCamera();
	}

	this->renderer_->Render();
	this->renderer_->GetRenderWindow()->Render();
}

//-----------------------------------------------------------------------------
void Viewer::set_opacity(float opacity)
{
	foreach(vtkSmartPointer<vtkActor> actor, this->surface_actors_) {
		actor->GetProperty()->SetOpacity(opacity);
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
void Viewer::set_clipping_plane(bool clip)
{
	if (!clip)
	{

		for (int i = 0; i < this->surface_mappers_.size(); i++)
		{
			this->surface_mappers_[i]->RemoveAllClippingPlanes();
		}

		if (this->plane_widget_)
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
		imp_plane_rep_->SetPlaceFactor(1.25); // This must be set prior to placing the widget

		double bounds[6];
		this->renderer_->ComputeVisiblePropBounds(bounds);

		double center[3];
		center[0] = (bounds[0] + bounds[1]) / 2.0;
		center[1] = (bounds[2] + bounds[3]) / 2.0;
		center[2] = (bounds[4] + bounds[5]) / 2.0;

		imp_plane_rep_->PlaceWidget(bounds);
		imp_plane_rep_->SetNormal(plane->GetNormal());
		imp_plane_rep_->SetOrigin(center);
		imp_plane_rep_->GetPlane(this->plane);

		for (int i = 0; i < this->surface_mappers_.size(); i++)
		{
			this->surface_mappers_[i]->AddClippingPlane(plane);
		}

		plane_widget_ = vtkSmartPointer<vtkImplicitPlaneWidget2>::New();
		plane_widget_->SetInteractor(this->renderer_->GetRenderWindow()->GetInteractor());
		plane_widget_->SetRepresentation(imp_plane_rep_);
		plane_widget_->AddObserver(vtkCommand::InteractionEvent, callback_);
		plane_widget_->On();
	}

	this->redraw();
}


//-----------------------------------------------------------------------------
vtkSmartPointer<vtkPolyData> Viewer::scale_mesh(QSharedPointer<Structure> s)
{
	vtkSmartPointer<vtkPolyData> mesh = s->get_mesh_tubes();

	if (s->get_type() == 1)
	{
		return mesh;
	}

	// else it's a child structure, scale it
	double scale = Preferences::Instance().get_child_scale();
	if (scale == 1.0)
	{
		return mesh;
	}

	double center[3];
	mesh->GetCenter(center);

	vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
	transform->Translate(center[0], center[1], center[2]);
	transform->Scale(scale, scale, scale);
	transform->Translate(-center[0], -center[1], -center[2]);

	vtkSmartPointer<vtkTransformPolyDataFilter> transform_filter =
		vtkSmartPointer<vtkTransformPolyDataFilter>::New();
	transform_filter->SetInputData(mesh);
	transform_filter->SetTransform(transform);
	transform_filter->Update();
	mesh = transform_filter->GetOutput();
	return mesh;
}
