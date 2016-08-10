#include <Data/Structure.h>
#include <Data/Json.h>
//#include <Data/PointSampler.h>
//#include <Data/AlphaShape.h>
//#include <Data/FixedAlphaShape.h>

#include <vtkCenterOfMass.h>
#include <vtkMassProperties.h>
#include <vtkWindowedSincPolyDataFilter.h>
#include <vtkCleanPolyData.h>
#include <vtkSTLWriter.h>
#include <vtkFeatureEdges.h>
#include <vtkCellArray.h>
#include <vtkFillHolesFilter.h>
#include <vtkPolyDataNormals.h>
#include <vtkSmoothPolyDataFilter.h>
#include <vtkTriangleFilter.h>
//#include <vtkFeatureVertices.h>
#include <vtkLoopSubdivisionFilter.h>
#include <vtkLinearSubdivisionFilter.h>
#include <vtkLineSource.h>
#include <vtkWindowedSincPolyDataFilter.h>
#include <vtkRegularPolygonSource.h>
//#include <vtkPLYWriter.h>
#include <vtkCardinalSpline.h>

#include <vtkPolyDataWriter.h>
#include <vtkBooleanOperationPolyDataFilter.h>
#include <vtkAppendPolyData.h>
#include <vtkTubeFilter.h>
#include <vtkDoubleArray.h>
#include <vtkPointData.h>

#include <vtkSphereSource.h>
#include <vtkTriangle.h>
#include <vtkMath.h>
#include <vtkDecimatePro.h>
#include <vtkTupleInterpolator.h>

#include <vtkParametricSpline.h>
#include <vtkParametricFunctionSource.h>

#include <Visualization/customQuadricDecimation.h>

#include <vtkButterflySubdivisionFilter.h>

#include <QVariant>

//#include <CGAL/IO/Polyhedron_iostream.h>
//#include <CGAL/Inverse_index.h>
//#include <CGAL/make_skin_surface_mesh_3.h>

//#include <CGAL/Polyhe>

//-----------------------------------------------------------------------------
Structure::Structure()
{
  this->color_ = QColor( 128 + ( qrand() % 128 ), 128 + ( qrand() % 128 ), 128 + ( qrand() % 128 ) );
  this->num_tubes_ = 0;
}

//-----------------------------------------------------------------------------
Structure::~Structure()
{}

//-----------------------------------------------------------------------------
QSharedPointer<StructureHash> Structure::create_structures( QList<QVariant> structure_list,
                                                            QList<QVariant> location_list,
                                                            QList<QVariant> link_list )
{

  QSharedPointer<StructureHash> structures = QSharedPointer<StructureHash> ( new StructureHash() );

  QHash<int, QSharedPointer<NodeMap> > node_maps;

  foreach( QVariant var, structure_list ) {
    QMap<QString, QVariant> item = var.toMap();
    int id = item["ID"].toLongLong();
    int type = item["TypeID"].toInt();
    QSharedPointer<Structure> structure = QSharedPointer<Structure>( new Structure() );
    structure->id_ = id;
    structure->type_ = type;
    structures->insert( id, structure );
  }

  std::cerr << "structure list length: " << structure_list.size() << "\n";
  std::cerr << "location list length: " << location_list.size() << "\n";
  std::cerr << "link list length: " << link_list.size() << "\n";

  float units_per_pixel = 2.18 / 1000.0;
  float units_per_section = -( 90.0 / 1000.0 );

  NodeMap full_node_map;

  // construct nodes
  foreach( QVariant var, location_list ) {
    QSharedPointer<Node> n = QSharedPointer<Node>( new Node() );
    QMap<QString, QVariant> item = var.toMap();
    n->id = item["ID"].toLongLong();
    n->x = item["VolumeX"].toDouble();
    n->y = item["VolumeY"].toDouble();
    n->z = item["Z"].toDouble();
    n->radius = item["Radius"].toDouble();
    n->parent_id = item["ParentID"].toLongLong();
    n->graph_id = -1;

    // scale
    n->x = n->x * units_per_pixel;
    n->y = n->y * units_per_pixel;
    n->z = n->z * units_per_section;
    n->radius = n->radius * units_per_pixel;

    if ( !structures->contains( n->parent_id ) )
    {
      std::cerr << "Error: could not find structure: " << n->parent_id << "\n";
      return structures;
    }

    structures->value( n->parent_id )->node_map_[n->id] = n;
    full_node_map[n->id] = n;
  }

  foreach( QVariant var, link_list ) {
    Link link;
    QMap<QString, QVariant> item = var.toMap();

    link.a = item["A"].toLongLong();
    link.b = item["B"].toLongLong();

    if ( !full_node_map.contains( link.a ) || !full_node_map.contains( link.b ) )
    {
      continue;
    }

    full_node_map[link.a]->linked_nodes.append( link.b );
    full_node_map[link.b]->linked_nodes.append( link.a );

    if ( full_node_map[link.a]->parent_id != full_node_map[link.b]->parent_id )
    {
      std::cerr << "links can go between structs?!\n";
    }

    long parent_id = full_node_map[link.a]->parent_id;
    structures->value( parent_id )->links_.append( link );
  }

  foreach( QSharedPointer<Structure> structure, structures->values() ) {
    //std::cerr << "===Initial===\n";
    //structure->link_report();

    structure->connect_subgraphs();

    //std::cerr << "===After graph connection===\n";
    //structure->link_report();

    //std::cerr << "number of nodes : " << structure->node_map_.size() << "\n";

	structure->cull_outliers();

    //structure->cull_locations();

    //structure->connect_subgraphs();

    //std::cerr << "===After location culling===\n";
    //structure->link_report();
  }

  return structures;
}

//-----------------------------------------------------------------------------
QSharedPointer<Structure> Structure::create_structure( int id, QList<QVariant> structure_list,
                                                       QList<QVariant> location_list, QList<QVariant> link_list )
{

  QSharedPointer<Structure> structure = QSharedPointer<Structure>( new Structure() );
  structure->id_ = id;

  float units_per_pixel = 2.18 / 1000.0;
  float units_per_section = -( 90.0 / 1000.0 );

  std::cerr << "structure list length: " << structure_list.size() << "\n";
  std::cerr << "location list length: " << location_list.size() << "\n";
  std::cerr << "link list length: " << link_list.size() << "\n";

  // construct nodes
  foreach( QVariant var, location_list ) {
    QSharedPointer<Node> n = QSharedPointer<Node>( new Node() );
    QMap<QString, QVariant> item = var.toMap();
    n->id = item["ID"].toLongLong();
    n->x = item["VolumeX"].toDouble();
    n->y = item["VolumeY"].toDouble();
    n->z = item["Z"].toDouble();
    n->radius = item["Radius"].toDouble();
    n->parent_id = item["ParentID"].toLongLong();
    n->graph_id = -1;

    // scale
    n->x = n->x * units_per_pixel;
    n->y = n->y * units_per_pixel;
    n->z = n->z * units_per_section;
    n->radius = n->radius * units_per_pixel;

    if ( item["ParentID"] == id )
    {
      structure->node_map_[n->id] = n;
    }
  }

  std::cerr << "Found " << structure->node_map_.size() << " nodes\n";

  foreach( QVariant var, link_list ) {
    Link link;
    QMap<QString, QVariant> item = var.toMap();

    link.a = item["A"].toLongLong();
    link.b = item["B"].toLongLong();

    if ( structure->node_map_.find( link.a ) == structure->node_map_.end()
         || structure->node_map_.find( link.b ) == structure->node_map_.end() )
    {
      continue;
    }

    structure->node_map_[link.a]->linked_nodes.append( link.b );
    structure->node_map_[link.b]->linked_nodes.append( link.a );
    structure->links_.append( link );
  }

  std::cerr << "Found " << structure->links_.size() << " links\n";

  std::cerr << "===Initial===\n";
  structure->link_report();

  structure->connect_subgraphs();

  std::cerr << "===After graph connection===\n";
  structure->link_report();

  std::cerr << "number of nodes : " << structure->node_map_.size() << "\n";

  structure->cull_locations();

  structure->connect_subgraphs();

  std::cerr << "===After location culling===\n";
  structure->link_report();
  return structure;
}

//-----------------------------------------------------------------------------
NodeMap Structure::get_node_map()
{
  return this->node_map_;
}

/*
   //-----------------------------------------------------------------------------
   vtkSmartPointer<vtkPolyData> Structure::get_mesh_old()
   {
   if ( this->mesh_ )
   {
    return this->mesh_;
   }

   NodeMap node_map = this->get_node_map();

   std::list<Point> points;

   vtkSmartPointer<vtkPolyData> poly_data = vtkSmartPointer<vtkPolyData>::New();

   bool first = true;

   // spheres
   for ( NodeMap::iterator it = node_map.begin(); it != node_map.end(); ++it )
   {

    QSharedPointer<Node> n = it.value();

    if ( n->linked_nodes.size() != 1 )
    {
      continue;
    }

   //    std::cerr << "adding sphere: " << n.id << "(" << n.x << "," << n.y << "," << n.z << "," << n.radius << ")\n";

    vtkSmartPointer<vtkSphereSource> sphere = vtkSmartPointer<vtkSphereSource>::New();
    sphere->SetCenter( n->x, n->y, n->z );
    sphere->SetRadius( n->radius );
    sphere->Update();

    if ( first )
    {
      poly_data = sphere->GetOutput();
      first = false;
    }
    else
    {

      vtkSmartPointer<vtkBooleanOperationPolyDataFilter> booleanOperation =
        vtkSmartPointer<vtkBooleanOperationPolyDataFilter>::New();
      booleanOperation->SetOperationToUnion();

      booleanOperation->SetInputData( 0, poly_data );
      booleanOperation->SetInputData( 1, sphere->GetOutput() );
      booleanOperation->Update();
      poly_data = booleanOperation->GetOutput();

    }
   }


   foreach( Link link, this->get_links() ) {

    if ( node_map.find( link.a ) == node_map.end() || node_map.find( link.b ) == node_map.end() )
    {
      continue;
    }

    QSharedPointer<Node> n1 = node_map[link.a];
    QSharedPointer<Node> n2 = node_map[link.b];

    vtkSmartPointer<vtkPoints> vtk_points = vtkSmartPointer<vtkPoints>::New();

    vtk_points->InsertNextPoint( n1->x, n1->y, n1->z );
    vtk_points->InsertNextPoint( n2->x, n2->y, n2->z );

    vtkSmartPointer<vtkCellArray> lines =
      vtkSmartPointer<vtkCellArray>::New();
    lines->InsertNextCell( 2 );
    lines->InsertCellPoint( 0 );
    lines->InsertCellPoint( 1 );

    vtkSmartPointer<vtkPolyData> polyData =
      vtkSmartPointer<vtkPolyData>::New();
    polyData->SetPoints( vtk_points );
    polyData->SetLines( lines );

    vtkSmartPointer<vtkTubeFilter> tube
      = vtkSmartPointer<vtkTubeFilter>::New();
    tube->SetInputData( polyData );
    tube->CappingOn();
    tube->SetRadius( n1->radius );
    tube->SetNumberOfSides( 10 );
    tube->Update();

    vtkSmartPointer<vtkAppendPolyData> append = vtkSmartPointer<vtkAppendPolyData>::New();
    append->AddInputData( poly_data );
    append->AddInputData( tube->GetOutput() );
    append->Update();
    poly_data = append->GetOutput();
   }

   this->mesh_ = poly_data;

   return this->mesh_;
   }
 */

/*
   //-----------------------------------------------------------------------------
   vtkSmartPointer<vtkPolyData> Structure::get_mesh_alpha()
   {
   if ( !this->mesh_ )
   {
    PointSampler ps( this );
    std::list<Point> points = ps.sample_points();

    AlphaShape alpha_shape;
    alpha_shape.set_points( points );
    vtkSmartPointer<vtkPolyData> poly_data = alpha_shape.get_mesh();

    //FixedAlphaShape alpha_shape;
    //alpha_shape.set_points( points );
    //vtkSmartPointer<vtkPolyData> poly_data = alpha_shape.get_mesh();

    // clean
    std::cerr << "Number of points before cleaning: " << poly_data->GetNumberOfPoints() << "\n";
    vtkSmartPointer<vtkCleanPolyData> clean = vtkSmartPointer<vtkCleanPolyData>::New();
    clean->SetInputData( poly_data );
    clean->SetTolerance( 0.00001 );
    clean->Update();
    poly_data = clean->GetOutput();
    std::cerr << "Number of points after cleaning: " << poly_data->GetNumberOfPoints() << "\n";

    vtkSmartPointer<vtkFeatureEdges> features = vtkSmartPointer<vtkFeatureEdges>::New();
    features->SetInputData( poly_data );
    features->NonManifoldEdgesOn();
    features->BoundaryEdgesOff();
    features->FeatureEdgesOff();
    features->Update();

    vtkSmartPointer<vtkPolyData> nonmanifold = features->GetOutput();

    std::cerr << "Number of non-manifold points: " << nonmanifold->GetNumberOfPoints() << "\n";
    std::cerr << "Number of non-manifold cells: " << nonmanifold->GetNumberOfCells() << "\n";

    std::vector<int> remove;

    for ( int j = 0; j < poly_data->GetNumberOfPoints(); j++ )
    {
      double p2[3];
      poly_data->GetPoint( j, p2 );

      for ( int i = 0; i < nonmanifold->GetNumberOfPoints(); i++ )
      {
        double p[3];
        nonmanifold->GetPoint( i, p );

        if ( p[0] == p2[0] && p[1] == p2[1] && p[2] == p2[2] )
        {
          remove.push_back( j );
        }
      }
    }

    std::cerr << "Removing " << remove.size() << " non-manifold vertices\n";

    vtkSmartPointer<vtkPolyData> new_poly_data = vtkSmartPointer<vtkPolyData>::New();
    vtkSmartPointer<vtkPoints> vtk_pts = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkCellArray> vtk_triangles = vtkSmartPointer<vtkCellArray>::New();

    for ( int i = 0; i < poly_data->GetNumberOfCells(); i++ )
    {
      vtkSmartPointer<vtkIdList> list = vtkIdList::New();
      poly_data->GetCellPoints( i, list );

      bool match = false;
      for ( int j = 0; j < list->GetNumberOfIds(); j++ )
      {
        int id = list->GetId( j );
        for ( unsigned int k = 0; k < remove.size(); k++ )
        {
          if ( id == remove[k] )
          {
            match = true;
          }
        }
      }

      if ( match )
      {
        poly_data->DeleteCell( i );
      }
    }

    poly_data->RemoveDeletedCells();

    features = vtkSmartPointer<vtkFeatureEdges>::New();
    features->SetInputData( poly_data );
    features->NonManifoldEdgesOn();
    features->BoundaryEdgesOff();
    features->FeatureEdgesOff();
    features->Update();

    nonmanifold = features->GetOutput();

    //std::cerr < "Number of non-manifold points: " << nonmanifold->GetNumberOfPoints() << "\n";
    //std::cerr << "Number of non-manifold cells: " << nonmanifold->GetNumberOfCells() << "\n";

    vtkSmartPointer< vtkTriangleFilter > triangle_filter =
      vtkSmartPointer< vtkTriangleFilter >::New();
    triangle_filter->SetInputData( poly_data );
    triangle_filter->Update();
    poly_data = triangle_filter->GetOutput();

    vtkSmartPointer<vtkSTLWriter> writer = vtkSmartPointer<vtkSTLWriter>::New();
    writer->SetFileName( "Z:\\shared\\file.stl" );
    writer->SetInputData( poly_data );
    writer->Write();

    // clean
    clean = vtkSmartPointer<vtkCleanPolyData>::New();
    clean->SetInputData( poly_data );
    clean->Update();
    poly_data = clean->GetOutput();

    // smooth


    vtkSmartPointer<vtkLoopSubdivisionFilter> subdivision = vtkSmartPointer<vtkLoopSubdivisionFilter>::New();
    subdivision->SetInputData( poly_data );
    subdivision->SetNumberOfSubdivisions( 2 );
    subdivision->Update();
    poly_data = subdivision->GetOutput();

    // Make the triangle winding order consistent
    vtkSmartPointer<vtkPolyDataNormals> normals =
      vtkSmartPointer<vtkPolyDataNormals>::New();
    normals->SetInputData( poly_data );
    normals->ConsistencyOn();
    normals->SplittingOff();
    normals->Update();
    poly_data = normals->GetOutput();


    // Make the triangle winding order consistent
    normals = vtkSmartPointer<vtkPolyDataNormals>::New();
    normals->SetInputData( poly_data );
    normals->ConsistencyOn();
    normals->SplittingOff();
    normals->Update();
    poly_data = normals->GetOutput();

    this->mesh_ = poly_data;
   }

   return this->mesh_;
   }
 */
//-----------------------------------------------------------------------------
int Structure::get_id()
{
  return this->id_;
}

//-----------------------------------------------------------------------------
int Structure::get_type()
{
  return this->type_;
}

//-----------------------------------------------------------------------------
double Structure::get_volume()
{
  return 0;
  /*
  vtkSmartPointer<vtkPolyData> mesh = this->get_mesh_union();

  vtkSmartPointer<vtkMassProperties> mass_properties = vtkSmartPointer<vtkMassProperties>::New();

  mass_properties->SetInputData( mesh );
  mass_properties->Update();

  return mass_properties->GetVolume();
  */
}

//-----------------------------------------------------------------------------
QString Structure::get_center_of_mass_string()
{
  return "";
  /*
  vtkSmartPointer<vtkPolyData> mesh = this->get_mesh_union();

  // Compute the center of mass
  vtkSmartPointer<vtkCenterOfMass> center_of_mass =
    vtkSmartPointer<vtkCenterOfMass>::New();
  center_of_mass->SetInputData( mesh );
  center_of_mass->SetUseScalarsAsWeights( false );
  center_of_mass->Update();

  double center[3];
  center_of_mass->GetCenter( center );

  QString str = QString::number( center[0] ) + ", " + QString::number( center[1] ) + ", " + QString::number( center[2] );

  return str;
  */
}

//-----------------------------------------------------------------------------
QList<Link> Structure::get_links()
{
  return this->links_;
}

//-----------------------------------------------------------------------------
void Structure::set_color( QColor color )
{
  this->color_ = color;
}

//-----------------------------------------------------------------------------
QColor Structure::get_color()
{
  return this->color_;
}

//-----------------------------------------------------------------------------
vtkSmartPointer<vtkPolyData> Structure::recopy_mesh( vtkSmartPointer<vtkPolyData> mesh )
{

  vtkSmartPointer<vtkPolyData> poly_data = vtkSmartPointer<vtkPolyData>::New();
  vtkSmartPointer<vtkPoints> vtk_pts = vtkSmartPointer<vtkPoints>::New();
  vtkSmartPointer<vtkCellArray> vtk_triangles = vtkSmartPointer<vtkCellArray>::New();

  for ( int i = 0; i < mesh->GetNumberOfPoints(); i++ )
  {
    vtk_pts->InsertNextPoint( mesh->GetPoint( i ) );
  }

  for ( int i = 0; i < mesh->GetNumberOfCells(); i++ )
  {
    if ( mesh->GetCell( i )->GetNumberOfPoints() == 3 )
    {
      vtk_triangles->InsertNextCell( mesh->GetCell( i ) );
    }
  }

  poly_data->SetPoints( vtk_pts );
  poly_data->SetPolys( vtk_triangles );

  return poly_data;
}

//-----------------------------------------------------------------------------
vtkSmartPointer<vtkPolyData> Structure::get_mesh_parts()
{
  if ( this->mesh_ )
  {
    return this->mesh_;
  }

  //std::cerr << "creating mesh...\n";

  NodeMap node_map = this->get_node_map();

  //std::list<Point> points;

  vtkSmartPointer<vtkPolyData> poly_data = vtkSmartPointer<vtkPolyData>::New();

  bool first = true;

  // spheres
  for ( NodeMap::iterator it = node_map.begin(); it != node_map.end(); ++it )
  {

    QSharedPointer<Node> n = it.value();

    if ( n->linked_nodes.size() != 1 )
    {
      continue;
    }

//    std::cerr << "adding sphere: " << n.id << "(" << n.x << "," << n.y << "," << n.z << "," << n.radius << ")\n";

    vtkSmartPointer<vtkSphereSource> sphere = vtkSmartPointer<vtkSphereSource>::New();
    sphere->SetCenter( n->x, n->y, n->z );
    sphere->SetRadius( n->radius );
    sphere->Update();

    if ( first )
    {
      poly_data = sphere->GetOutput();
      first = false;
    }
    else
    {
/*
      vtkSmartPointer<vtkBooleanOperationPolyDataFilter> booleanOperation =
        vtkSmartPointer<vtkBooleanOperationPolyDataFilter>::New();
      booleanOperation->SetOperationToUnion();

      booleanOperation->SetInputData( 0, poly_data );
      booleanOperation->SetInputData( 1, sphere->GetOutput() );
      booleanOperation->Update();
      poly_data = booleanOperation->GetOutput();
 */

      vtkSmartPointer<vtkAppendPolyData> append = vtkSmartPointer<vtkAppendPolyData>::New();
      append->AddInputData( poly_data );
      append->AddInputData( sphere->GetOutput() );
      append->Update();
      poly_data = append->GetOutput();
    }
  }

  foreach( Link link, this->get_links() ) {

    if ( node_map.find( link.a ) == node_map.end() || node_map.find( link.b ) == node_map.end() )
    {
      continue;
    }

    QSharedPointer<Node> n1 = node_map[link.a];
    QSharedPointer<Node> n2 = node_map[link.b];

    vtkSmartPointer<vtkPoints> vtk_points = vtkSmartPointer<vtkPoints>::New();

    vtk_points->InsertNextPoint( n1->x, n1->y, n1->z );
    vtk_points->InsertNextPoint( n2->x, n2->y, n2->z );

    vtkSmartPointer<vtkCellArray> lines = vtkSmartPointer<vtkCellArray>::New();
    lines->InsertNextCell( 2 );
    lines->InsertCellPoint( 0 );
    lines->InsertCellPoint( 1 );

    vtkSmartPointer<vtkDoubleArray> tube_radius = vtkSmartPointer<vtkDoubleArray>::New();
    tube_radius->SetName( "tube_radius" );
    tube_radius->SetNumberOfTuples( 2 );
    tube_radius->SetTuple1( 0, n1->radius );
    tube_radius->SetTuple1( 1, n2->radius );

    vtkSmartPointer<vtkPolyData> poly_data = vtkSmartPointer<vtkPolyData>::New();
    poly_data->SetPoints( vtk_points );
    poly_data->SetLines( lines );
    poly_data->GetPointData()->AddArray( tube_radius );
    poly_data->GetPointData()->SetActiveScalars( "tube_radius" );

    vtkSmartPointer<vtkTubeFilter> tube = vtkSmartPointer<vtkTubeFilter>::New();
    tube->SetInputData( poly_data );
    tube->CappingOn();
    tube->SetVaryRadiusToVaryRadiusByAbsoluteScalar();
    tube->SetRadius( n1->radius );
    tube->SetNumberOfSides( 20 );
    tube->Update();

    vtkSmartPointer<vtkAppendPolyData> append = vtkSmartPointer<vtkAppendPolyData>::New();
    append->AddInputData( poly_data );
    append->AddInputData( tube->GetOutput() );
    append->Update();
    poly_data = append->GetOutput();
  }

  this->mesh_ = poly_data;

  return this->mesh_;
}

//-----------------------------------------------------------------------------
double Structure::distance( const QSharedPointer<Node> &n1, const QSharedPointer<Node> &n2 )
{
  double squared_dist = ( n1->x - n2->x ) * ( n1->x - n2->x )
                        + ( n1->y - n2->y ) * ( n1->y - n2->y )
                        + ( n1->z - n2->z ) * ( n1->z - n2->z );
  return sqrt( squared_dist );
}

//-----------------------------------------------------------------------------
void Structure::connect_subgraphs()
{
  long max_count = 0;

  // initialize
  for ( NodeMap::iterator it = this->node_map_.begin(); it != this->node_map_.end(); ++it )
  {
    it.value()->graph_id = -1;
  }

  for ( NodeMap::iterator it = this->node_map_.begin(); it != this->node_map_.end(); ++it )
  {
    QSharedPointer<Node> n = it.value();

    if ( n->graph_id == -1 )
    {
      max_count++;
      n->graph_id = max_count;
      this->node_map_[it.key()] = n;

      QList<int> connections = n->linked_nodes;

      while ( connections.size() > 0 )
      {
        int node = connections.first();
        connections.pop_front();

        QSharedPointer<Node> child = this->node_map_[node];

        if ( child->graph_id == -1 )
        {
          child->graph_id = max_count;
          connections.append( child->linked_nodes );
          //this->node_map_[node] = child;  // write back
        }
      }
    }
  }

  //std::cerr << "Found " << max_count << " graphs\n";

  // create links between graphs

  QList<int> primary_group;

  for ( NodeMap::iterator it = this->node_map_.begin(); it != this->node_map_.end(); ++it )
  {
    QSharedPointer<Node> n = it.value();
    if ( n->graph_id == 1 )
    {
      primary_group.append( n->id );
    }
  }

  for ( int i = 2; i <= max_count; i++ )
  {

    // find closest pair
    double min_dist = DBL_MAX;
    int primary_id = -1;
    int child_id = -1;

    for ( NodeMap::iterator it = this->node_map_.begin(); it != this->node_map_.end(); ++it )
    {
      QSharedPointer<Node> n = it.value();

      if ( n->graph_id == i )
      {

        for ( NodeMap::iterator it2 = this->node_map_.begin(); it2 != this->node_map_.end(); ++it2 )
        {
          QSharedPointer<Node> pn = it2.value();
          if ( pn->graph_id >= i )
          {
            continue;
          }

          double point1[3], point2[3];
          point1[0] = n->x;
          point1[1] = n->y;
          point1[2] = n->z;
          point2[0] = pn->x;
          point2[1] = pn->y;
          point2[2] = pn->z;
          double distance = sqrt( vtkMath::Distance2BetweenPoints( point1, point2 ) );

          if ( distance < min_dist )
          {
            min_dist = distance;
            primary_id = pn->id;
            child_id = n->id;
          }
        }
      }
    }

    Link new_link;
    new_link.a = primary_id;
    new_link.b = child_id;
    this->links_.append( new_link );

    this->node_map_[primary_id]->linked_nodes.append( child_id );
    this->node_map_[child_id]->linked_nodes.append( primary_id );
  }
}

void Structure::remove_node(long id)
{
	QSharedPointer<Node> toRemove = this->node_map_[id];
	for (QList<int>::iterator it = toRemove->linked_nodes.begin(); it != toRemove->linked_nodes.end(); ++it)
	{
		QSharedPointer<Node> linkedNode = this->node_map_[*it];
		linkedNode->linked_nodes.removeAll(id); //Remove link to node we are deleting
		linkedNode->linked_nodes.append(toRemove->linked_nodes);
		linkedNode->linked_nodes.removeAll(linkedNode->id); //Remove circular link
	}

	this->node_map_.remove(id); 
}

/*------------------------------------------------------------------------------
Occasionally a section will be out of alignment and annotations are placed far away from the correct position at a particular Z level.
To correct this we remove nodes whose linked nodes are closer to each other than the node itself. 

Conceptually this is the difference between three points making a roughly straight line or a triangle.  If it looks like a triangle we want one of the nodes removed.
*/
void Structure::cull_outliers()
{
	QList<long> node_id_list = this->node_map_.keys(); 
	for (QList<long>::iterator id = node_id_list.begin(); id != node_id_list.end(); ++id)
	{  
		QSharedPointer<Node> n = this->node_map_[*id];
		if (n->IsBranch() || n->IsEndpoint())
		{
			continue;
		}

		// if the two other locations are closer together than this one is to either of them

		QSharedPointer<Node> node_a = this->node_map_[n->linked_nodes[0]];
		QSharedPointer<Node> node_b = this->node_map_[n->linked_nodes[1]];

		double min_dist = std::min(distance(n, node_a), distance(n, node_b));

		if (distance(node_a, node_b) < min_dist)
		{
			std::cerr << "removed outlier! " << n->id << "\n";
			this->remove_node(*id);
		}
	}
}

//-----------------------------------------------------------------------------
void Structure::cull_locations()
{

  std::vector<int> remove_list;
  do
  {
    remove_list.clear();
    // cull overlapping locations
    for ( NodeMap::iterator it = this->node_map_.begin(); it != this->node_map_.end(); ++it )
    {
      QSharedPointer<Node> n = it.value();

      if ( n->linked_nodes.size() != 2 )
      {
        continue;
      }

      bool removed = false;

      int other_id = -1;

      if ( n->linked_nodes.size() == 2 )
      {
        // if the two other locations are closer together than this one is to either of them

        QSharedPointer<Node> node_a = this->node_map_[n->linked_nodes[0]];
        QSharedPointer<Node> node_b = this->node_map_[n->linked_nodes[1]];

        double min_dist = std::min( distance( n, node_a ), distance( n, node_b ) );

        if ( distance( node_a, node_b ) < min_dist )
        {
          //std::cerr << "removed outlier!\n";
          removed = true;
          remove_list.push_back( n->id );
          other_id = n->linked_nodes[0];
        }
      }

      foreach( int id, n->linked_nodes ) {

        if ( !removed )
        {
          QSharedPointer<Node> other = this->node_map_[id];

          if ( other->linked_nodes.size() <= n->linked_nodes.size() )  // remove the one with less links
          {
            if ( distance( n, other ) < std::max( n->radius, other->radius ) )
            {
              remove_list.push_back( n->id );
              other_id = other->id;
              removed = true;
            }
          }
        }
      }

      if ( removed )
      {
        this->node_map_[other_id]->linked_nodes.removeOne( n->id );

        foreach( int id, n->linked_nodes ) {

          if ( id != other_id )
          {
            this->node_map_[other_id]->linked_nodes.append( id );
            this->node_map_[id]->linked_nodes.removeOne( n->id );
            this->node_map_[id]->linked_nodes.append( other_id );
          }
        }
      }
    }

    //std::cerr << "remove list size : " << remove_list.size() << "\n";

    for ( unsigned int i = 0; i < remove_list.size(); i++ )
    {
      this->node_map_.remove( remove_list[i] );
    }

    this->connect_subgraphs();
  }
  while ( remove_list.size() > 0 );
}

//-----------------------------------------------------------------------------
void Structure::link_report()
{
  std::vector<int> link_counts( 100 );

  for ( NodeMap::iterator it = this->node_map_.begin(); it != this->node_map_.end(); ++it )
  {
    QSharedPointer<Node> n = it.value();
    link_counts[n->linked_nodes.size()]++;
  }

  for ( int i = 0; i < 100; i++ )
  {
    if ( link_counts[i] > 0 )
    {
      std::cerr << "Nodes with " << i << " links: " << link_counts[i] << "\n";
    }
  }
}

//-----------------------------------------------------------------------------
vtkSmartPointer<vtkPolyData> Structure::get_mesh_tubes()
{
  if ( this->mesh_ )
  {
    return this->mesh_;
  }

  //std::cerr << "creating mesh...\n";

  //NodeMap node_map = this->get_node_map();

  //std::list<Point> points;

  vtkSmartPointer<vtkPolyData> poly_data = vtkSmartPointer<vtkPolyData>::New();

  // reset visited
  for ( NodeMap::iterator it = this->node_map_.begin(); it != this->node_map_.end(); ++it )
  {
    it.value()->visited = false;
  }

  int root = -1;
  // find a dead-end
  for ( NodeMap::iterator it = this->node_map_.begin(); it != this->node_map_.end(); ++it )
  {
    QSharedPointer<Node> n = it.value();
    if ( n->linked_nodes.size() == 1 || n->linked_nodes.size() == 0 )
    {
      root = n->id;
      break;
    }
  }

  if ( root == -1 )
  {
    std::cerr << "Error: could not locate root node\n";
    return this->mesh_;
  }

  QSharedPointer<Node> n = this->node_map_[root];

  vtkSmartPointer<vtkAppendPolyData> append = vtkSmartPointer<vtkAppendPolyData>::New();

  this->add_polydata( n, -1, append, QList<int>() );

  //std::cerr << "Num of items: " << append->GetNumberOfInputConnections( 0 ) << "\n";

  //std::cerr << "number of tubes: " << this->num_tubes_ << "\n";

  append->Update();
  poly_data = append->GetOutput();

  //  std::cerr << "number of verts: " << poly_data->GetNumberOfPoints() << "\n";
  //std::cerr << "number of polys: " << poly_data->GetNumberOfCells() << "\n";

  vtkSmartPointer<vtkPolyDataNormals> normals = vtkSmartPointer<vtkPolyDataNormals>::New();
  normals->SetInputData( poly_data );
  normals->Update();
  poly_data = normals->GetOutput();

  this->mesh_ = poly_data;

  return this->mesh_;
}

//-----------------------------------------------------------------------------
void Structure::add_polydata( QSharedPointer<Node> n, int from, vtkSmartPointer<vtkAppendPolyData> append, QList<int> current_line )
{
  this->node_map_[n->id]->visited = true;

  if ( n->linked_nodes.size() == 2 )
  {
    current_line.append( n->id );

    for ( int i = 0; i < n->linked_nodes.size(); i++ )
    {
      QSharedPointer<Node> other = this->node_map_[n->linked_nodes[i]];
      if ( !other->visited )
      {
        this->add_polydata( other, n->id, append, current_line );
      }
    }
  }

  /*
     // add a circle at every location
     vtkSmartPointer<vtkRegularPolygonSource> circle = vtkSmartPointer<vtkRegularPolygonSource>::New();
     circle->GeneratePolygonOff();
     circle->SetNumberOfSides( 12 );
     circle->SetRadius( n->radius );
     circle->SetCenter( n->x, n->y, n->z );
     circle->Update();
     append->AddInputData( circle->GetOutput() );
     /**/

  if ( n->linked_nodes.size() != 2 )
  {

    /* sphere */
    vtkSmartPointer<vtkSphereSource> sphere = vtkSmartPointer<vtkSphereSource>::New();
    sphere->SetCenter( n->x, n->y, n->z );
    sphere->SetRadius( n->radius * 1.05 );
    //sphere->SetRadius( n.radius );

    int resolution = 10;
    if ( n->radius > 1.0 )
    {
      resolution = resolution * n->radius;
    }

    resolution = 15;

    sphere->SetPhiResolution( resolution );
    sphere->SetThetaResolution( resolution );
    sphere->Update();

    vtkSmartPointer<vtkPolyData> poly_data = vtkSmartPointer<vtkPolyData>::New();
    poly_data = sphere->GetOutput();

/*
    vtkSmartPointer<vtkUnsignedCharArray> colors =
      vtkSmartPointer<vtkUnsignedCharArray>::New();
    colors->SetNumberOfComponents( 3 );
    colors->SetName( "Colors" );

    int r = 128 + ( qrand() % 128 );
    int g = 128 + ( qrand() % 128 );
    int b = 128 + ( qrand() % 128 );
    for ( int i = 0; i < poly_data->GetNumberOfPoints(); ++i )
    {
      unsigned char tempColor[3] =
      {r, g, b};

      colors->InsertNextTupleValue( tempColor );
    }
    //poly_data->GetPointData()->SetScalars( colors );
 */

/*
    vtkSmartPointer< vtkTriangleFilter > triangle_filter = vtkSmartPointer< vtkTriangleFilter >::New();
    triangle_filter->SetInputData( poly_data );
    //    triangle_filter->PassLinesOff();
    triangle_filter->Update();
    poly_data = triangle_filter->GetOutput();

    std::cerr << "Number of points before cleaning: " << poly_data->GetNumberOfPoints() << "\n";
    vtkSmartPointer<vtkCleanPolyData> clean = vtkSmartPointer<vtkCleanPolyData>::New();
    clean->SetInputData( poly_data );
    //clean->SetTolerance( 0.00001 );
    clean->Update();
    poly_data = clean->GetOutput();
    std::cerr << "Number of points after cleaning: " << poly_data->GetNumberOfPoints() << "\n";

    this->num_tubes_++;
    //QString filename = QString("C:\\Users\\amorris\\part") + QString::number(this->num_tubes_) + ".ply";
    QString filename = QString("C:\\Users\\amorris\\part") + QString::number(this->num_tubes_) + ".vtk";
    vtkSmartPointer<vtkPolyDataWriter> writer4 = vtkSmartPointer<vtkPolyDataWriter>::New();
    //vtkSmartPointer<vtkPLYWriter> writer4 = vtkSmartPointer<vtkPLYWriter>::New();
    writer4->SetFileName( filename );
    writer4->SetInputData( poly_data );
    //writer4->SetFileTypeToBinary();
    writer4->Write();
 */

    append->AddInputData( poly_data );

    if ( current_line.size() > 0 )
    {
      current_line.append( n->id );

      vtkSmartPointer<vtkPoints> vtk_points = vtkSmartPointer<vtkPoints>::New();
      vtkSmartPointer<vtkCellArray> lines = vtkSmartPointer<vtkCellArray>::New();
      lines->InsertNextCell( current_line.size() );
      vtkSmartPointer<vtkDoubleArray> tube_radius_array = vtkSmartPointer<vtkDoubleArray>::New();
      tube_radius_array->SetName( "tube_radius" );

      vtkSmartPointer<vtkTupleInterpolator> interpolated_radius = vtkSmartPointer<vtkTupleInterpolator> ::New();
      interpolated_radius->SetInterpolationTypeToLinear();
      //interpolated_radius->SetInterpolationTypeToSpline();
      interpolated_radius->SetNumberOfComponents( 1 );

      int count = 0;
      foreach( int node_id, current_line ) {
        QSharedPointer<Node> node = this->node_map_[node_id];

        vtk_points->InsertNextPoint( node->x, node->y, node->z );
        lines->InsertCellPoint( count );
        tube_radius_array->InsertNextTuple1( node->radius );
        interpolated_radius->AddTuple( count, &( node->radius ) );
        count++;
      }

      vtkSmartPointer<vtkParametricSpline> spline = vtkSmartPointer<vtkParametricSpline>::New();
      //vtkSmartPointer<vtkCardinalSpline> spline = vtkSmartPointer<vtkCardinalSpline>::New();
      spline->SetPoints( vtk_points );

      // Interpolate the points
      vtkSmartPointer<vtkParametricFunctionSource> function_source =
        vtkSmartPointer<vtkParametricFunctionSource>::New();
      function_source->SetParametricFunction( spline );
      //function_source->SetUResolution( 30 * vtk_points->GetNumberOfPoints() );
      function_source->SetUResolution( 2 * vtk_points->GetNumberOfPoints() );
      //function_source->SetUResolution( vtk_points->GetNumberOfPoints() );

      function_source->Update();

      vtkSmartPointer<vtkPolyData> poly_data = vtkSmartPointer<vtkPolyData>::New();
      poly_data->SetPoints( vtk_points );
      poly_data->SetLines( lines );
      //poly_data->GetPointData()->AddArray( tube_radius_array );
      //poly_data->GetPointData()->SetActiveScalars( "tube_radius" );

      //append->AddInputData(function_source->GetOutput());
      // tmp: add line instead
      //append->AddInputData( poly_data );

      // Generate the radius scalars
      vtkSmartPointer<vtkDoubleArray> tube_radius = vtkSmartPointer<vtkDoubleArray>::New();
      unsigned int n = function_source->GetOutput()->GetNumberOfPoints();
      tube_radius->SetNumberOfTuples( n );
      tube_radius->SetName( "TubeRadius" );
      double tMin = interpolated_radius->GetMinimumT();
      double tMax = interpolated_radius->GetMaximumT();
      double radius;
      for ( unsigned int i = 0; i < n; ++i )
      {
        double t = ( tMax - tMin ) / ( n - 1 ) * i + tMin;
        interpolated_radius->InterpolateTuple( t, &radius );
        tube_radius->SetTuple1( i, radius );
      }

      // Add the scalars to the polydata
      vtkSmartPointer<vtkPolyData> tube_poly_data = vtkSmartPointer<vtkPolyData>::New();
      tube_poly_data = function_source->GetOutput();
      tube_poly_data->GetPointData()->AddArray( tube_radius );
      tube_poly_data->GetPointData()->SetActiveScalars( "TubeRadius" );

      vtkSmartPointer<vtkTubeFilter> tube = vtkSmartPointer<vtkTubeFilter>::New();
      //tube->SetInputData( poly_data );

      tube->SetInputData( tube_poly_data );

      tube->CappingOn();
      tube->SetVaryRadiusToVaryRadiusByAbsoluteScalar();

      //tube->SetRadius(n.radius);
      //tube->SetRadius( 0.1 );
      tube->SetNumberOfSides( 15 );
      tube->Update();

      poly_data = tube->GetOutput();

      /* //color
         vtkSmartPointer<vtkUnsignedCharArray> colors =
         vtkSmartPointer<vtkUnsignedCharArray>::New();
         colors->SetNumberOfComponents( 3 );
         colors->SetName( "Colors" );

         int r = 128 + ( qrand() % 128 );
         int g = 128 + ( qrand() % 128 );
         int b = 128 + ( qrand() % 128 );

         for ( int i = 0; i < poly_data->GetNumberOfPoints(); ++i )
         {
         unsigned char tempColor[3] =
         {r, g, b};

         colors->InsertNextTupleValue( tempColor );
         }

         //poly_data->GetPointData()->SetScalars( colors );
       */

      // here
      append->AddInputData( poly_data );

/*


      vtkSmartPointer< vtkTriangleFilter > triangle_filter = vtkSmartPointer< vtkTriangleFilter >::New();
      triangle_filter->SetInputData( poly_data );
      //    triangle_filter->PassLinesOff();
      triangle_filter->Update();
      poly_data = triangle_filter->GetOutput();

      std::cerr << "Number of points before cleaning: " << poly_data->GetNumberOfPoints() << "\n";
      vtkSmartPointer<vtkCleanPolyData> clean = vtkSmartPointer<vtkCleanPolyData>::New();
      clean->SetInputData( poly_data );
      //clean->SetTolerance( 0.00001 );
      clean->Update();
      poly_data = clean->GetOutput();
      std::cerr << "Number of points after cleaning: " << poly_data->GetNumberOfPoints() << "\n";



      this->num_tubes_++;
      //QString filename = QString("C:\\Users\\amorris\\part") + QString::number(this->num_tubes_) + ".ply";
      QString filename = QString("C:\\Users\\amorris\\part") + QString::number(this->num_tubes_) + ".vtk";
      //vtkSmartPointer<vtkPLYWriter> writer4 = vtkSmartPointer<vtkPLYWriter>::New();
      vtkSmartPointer<vtkPolyDataWriter> writer4 = vtkSmartPointer<vtkPolyDataWriter>::New();
      writer4->SetFileName( filename );
      writer4->SetInputData( poly_data );
      //writer4->SetFileTypeToBinary();
      writer4->Write();


      filename = QString("C:\\Users\\amorris\\part") + QString::number(this->num_tubes_) + ".stl";
      vtkSmartPointer<vtkSTLWriter> writer = vtkSmartPointer<vtkSTLWriter>::New();
      writer->SetFileName( filename );
      writer->SetInputData( poly_data );
      writer->Write();

 */
    }

    for ( int i = 0; i < n->linked_nodes.size(); i++ )
    {
      QSharedPointer<Node> other = this->node_map_[n->linked_nodes[i]];
      if ( !other->visited )
      {
        QList<int> new_line;
        new_line.append( n->id );
        this->add_polydata( other, n->id, append, new_line );
      }
    }
  }
}
