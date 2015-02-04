#include <Data/Structure.h>
#include <Data/Json.h>
#include <Data/PointSampler.h>
#include <Data/AlphaShape.h>

#include <vtkCenterOfMass.h>
#include <vtkMassProperties.h>

//-----------------------------------------------------------------------------
QSharedPointer<Structure> Structure::create_structure( int id, QString location_text, QString link_text )
{

  QSharedPointer<Structure> structure = QSharedPointer<Structure>( new Structure() );
  structure->id_ = id;

  QMap<QString, QVariant> map = Json::decode( location_text );
  QList<QVariant> location_list = map["value"].toList();

  map = Json::decode( link_text );
  QList<QVariant> link_list = map["value"].toList();

  float units_per_pixel = 2.18 / 1000.0;
  float units_per_section = 90.0 / 1000.0;

  // construct nodes
  foreach( QVariant var, location_list ) {
    Node n;
    QMap<QString, QVariant> item = var.toMap();
    n.x = item["VolumeX"].toDouble();
    n.y = item["VolumeY"].toDouble();
    n.z = item["Z"].toDouble();
    n.radius = item["Radius"].toDouble();
    n.id = item["ID"].toLongLong();

    n.x = n.x * units_per_pixel;
    n.y = n.y * units_per_pixel;
    n.z = n.z * units_per_section;
    n.radius = n.radius * units_per_pixel;

    structure->node_map_[n.id] = n;
  }

  return structure;
}

//-----------------------------------------------------------------------------
Structure::Structure()
{}

//-----------------------------------------------------------------------------
Structure::~Structure()
{}

//-----------------------------------------------------------------------------
NodeMap Structure::get_node_map()
{
  return this->node_map_;
}

//-----------------------------------------------------------------------------
vtkSmartPointer<vtkPolyData> Structure::get_mesh()
{
  if ( !this->mesh_ )
  {
    PointSampler ps( this );
    std::list<Point> points = ps.sample_points();

    AlphaShape alpha_shape;
    alpha_shape.set_points( points );
    this->mesh_ = alpha_shape.get_mesh();
  }

  return this->mesh_;
}

//-----------------------------------------------------------------------------
int Structure::get_id()
{
  return this->id_;
}

//-----------------------------------------------------------------------------
double Structure::get_volume()
{
  vtkSmartPointer<vtkPolyData> mesh = this->get_mesh();

  vtkSmartPointer<vtkMassProperties> mass_properties = vtkSmartPointer<vtkMassProperties>::New();

  mass_properties->SetInputData( mesh );
  mass_properties->Update();

  return mass_properties->GetVolume();
}

//-----------------------------------------------------------------------------
QString Structure::get_center_of_mass_string()
{
  vtkSmartPointer<vtkPolyData> mesh = this->get_mesh();

  // Compute the center of mass
  vtkSmartPointer<vtkCenterOfMass> center_of_mass =
    vtkSmartPointer<vtkCenterOfMass>::New();
  center_of_mass->SetInputData( mesh );
  center_of_mass->SetUseScalarsAsWeights( false );
  center_of_mass->Update();

  double center[3];
  center_of_mass->GetCenter(center);

  QString str = QString::number(center[0]) + ", " + QString::number(center[1]) + ", " + QString::number(center[2]);

  return str;

  
}
