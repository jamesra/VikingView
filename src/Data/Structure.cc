#include <Data/Structure.h>
#include <Data/Json.h>
#include <Data/PointSampler.h>
#include <Data/AlphaShape.h>
#include <Data/FixedAlphaShape.h>

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

#include <vtkBooleanOperationPolyDataFilter.h>
#include <vtkAppendPolyData.h>
#include <vtkTubeFilter.h>

#include <vtkTriangle.h>
#include <vtkMath.h>
#include <vtkDecimatePro.h>
#include <Visualization/customQuadricDecimation.h>

#include <vtkButterflySubdivisionFilter.h>

#include <CGAL/IO/Polyhedron_iostream.h>
#include <CGAL/Inverse_index.h>
#include <CGAL/make_skin_surface_mesh_3.h>

//#include <CGAL/Polyhe>

//-----------------------------------------------------------------------------
Structure::Structure()
{
  this->color_ = QColor( 128 + ( qrand() % 128 ), 128 + ( qrand() % 128 ), 128 + ( qrand() % 128 ) );
}

//-----------------------------------------------------------------------------
Structure::~Structure()
{}

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
  float units_per_section = -( 90.0 / 1000.0 );

  // construct nodes
  foreach( QVariant var, location_list ) {
    Node n;
    QMap<QString, QVariant> item = var.toMap();
    n.x = item["VolumeX"].toDouble();
    n.y = item["VolumeY"].toDouble();
    n.z = item["Z"].toDouble();
    n.radius = item["Radius"].toDouble();
    n.id = item["ID"].toLongLong();
    n.graph_id = -1;

    if ( n.z == 56 || n.z == 8 || n.z == 22 || n.z == 81 || n.z == 72 || n.z == 60 )
    {
      continue;
    }

    // scale
    n.x = n.x * units_per_pixel;
    n.y = n.y * units_per_pixel;
    n.z = n.z * units_per_section;
    n.radius = n.radius * units_per_pixel;

    structure->node_map_[n.id] = n;
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

    structure->node_map_[link.a].linked_nodes.append( link.b );
    structure->node_map_[link.b].linked_nodes.append( link.a );
    structure->links_.append( link );
  }

  std::cerr << "Found " << structure->links_.size() << " links\n";

  // identify all subgraphs

  long max_count = 0;

  for ( NodeMap::iterator it = structure->node_map_.begin(); it != structure->node_map_.end(); ++it )
  {
    Node n = it->second;

    if ( n.graph_id == -1 )
    {
      max_count++;
      n.graph_id = max_count;
      structure->node_map_[it->first] = n;

      QList<int> connections = n.linked_nodes;

      while ( connections.size() > 0 )
      {
        int node = connections.first();
        connections.pop_front();

        Node child = structure->node_map_[node];

        if ( child.graph_id == -1 )
        {
          child.graph_id = max_count;
          connections.append( child.linked_nodes );
          structure->node_map_[node] = child;  // write back
        }
      }
    }
  }

  std::cerr << "Found " << max_count << " graphs\n";

  // create links between graphs

  QList<int> primary_group;

  for ( NodeMap::iterator it = structure->node_map_.begin(); it != structure->node_map_.end(); ++it )
  {
    Node n = it->second;
    if ( n.graph_id == 1 )
    {
      primary_group.append( n.id );
    }
  }

  for ( int i = 2; i <= id; i++ )
  {

    // find closest pair
    double min_dist = DBL_MAX;
    int primary_id = -1;
    int child_id = -1;

    for ( NodeMap::iterator it = structure->node_map_.begin(); it != structure->node_map_.end(); ++it )
    {
      Node n = it->second;

      if ( n.graph_id == i )
      {

        for ( NodeMap::iterator it2 = structure->node_map_.begin(); it2 != structure->node_map_.end(); ++it2 )
        {
          Node pn = it2->second;
          if ( pn.graph_id >= i )
          {
            continue;
          }

          double point1[3], point2[3];
          point1[0] = n.x;
          point1[1] = n.y;
          point1[2] = n.z;
          point2[0] = pn.x;
          point2[1] = pn.y;
          point2[2] = pn.z;
          double distance = sqrt( vtkMath::Distance2BetweenPoints( point1, point2 ) );

          if ( distance < min_dist )
          {
            min_dist = distance;
            primary_id = pn.id;
            child_id = n.id;
          }
        }
      }
    }

    Link new_link;
    new_link.a = primary_id;
    new_link.b = child_id;
    structure->links_.append( new_link );
    //std::cerr << "added new link\n";
  }

  return structure;
}

//-----------------------------------------------------------------------------
NodeMap Structure::get_node_map()
{
  return this->node_map_;
}

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

    Node n = it->second;

    if ( n.linked_nodes.size() != 1 )
    {
      continue;
    }

//    std::cerr << "adding sphere: " << n.id << "(" << n.x << "," << n.y << "," << n.z << "," << n.radius << ")\n";

    vtkSmartPointer<vtkSphereSource> sphere = vtkSmartPointer<vtkSphereSource>::New();
    sphere->SetCenter( n.x, n.y, n.z );
    sphere->SetRadius( n.radius );
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

/*
   vtkSmartPointer<vtkAppendPolyData> append = vtkSmartPointer<vtkAppendPolyData>::New();
   append->AddInputData( poly_data );
   append->AddInputData( sphere->GetOutput() );
   append->Update();
   poly_data = append->GetOutput();
 */
    }
  }

/*

      vtkSmartPointer<vtkBooleanOperationPolyDataFilter> booleanOperation =
        vtkSmartPointer<vtkBooleanOperationPolyDataFilter>::New();
      booleanOperation->SetOperationToUnion();

      booleanOperation->SetInputData( 0, poly_data );
      booleanOperation->SetInputData( 1, poly_data );
      booleanOperation->Update();
      poly_data = booleanOperation->GetOutput();
 */

  foreach( Link link, this->get_links() ) {

    if ( node_map.find( link.a ) == node_map.end() || node_map.find( link.b ) == node_map.end() )
    {
      continue;
    }

    Node n1 = node_map[link.a];
    Node n2 = node_map[link.b];

    vtkSmartPointer<vtkPoints> vtk_points = vtkSmartPointer<vtkPoints>::New();

    vtk_points->InsertNextPoint( n1.x, n1.y, n1.z );
    vtk_points->InsertNextPoint( n2.x, n2.y, n2.z );

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
    tube->SetRadius( n1.radius );
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

//-----------------------------------------------------------------------------
vtkSmartPointer<vtkPolyData> Structure::get_mesh_old2()
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

    std::cerr << "Number of non-manifold points: " << nonmanifold->GetNumberOfPoints() << "\n";
    std::cerr << "Number of non-manifold cells: " << nonmanifold->GetNumberOfCells() << "\n";

    vtkSmartPointer< vtkTriangleFilter > triangle_filter =
      vtkSmartPointer< vtkTriangleFilter >::New();
    triangle_filter->SetInputData( poly_data );
    triangle_filter->Update();
    poly_data = triangle_filter->GetOutput();

    vtkSmartPointer<vtkSTLWriter> writer = vtkSmartPointer<vtkSTLWriter>::New();
    writer->SetFileName( "Z:\\shared\\file.stl" );
    writer->SetInputData( poly_data );
    writer->Write();

/*
    // fill holes
    vtkSmartPointer<vtkFillHolesFilter> fill_holes = vtkSmartPointer<vtkFillHolesFilter>::New();
    fill_holes->SetInputData( poly_data );
    fill_holes->SetHoleSize( 300 );
    fill_holes->Update();
    poly_data = fill_holes->GetOutput();



 */

    // clean
    clean = vtkSmartPointer<vtkCleanPolyData>::New();
    clean->SetInputData( poly_data );
    clean->Update();
    poly_data = clean->GetOutput();

    // smooth

/*

    vtkSmartPointer<vtkWindowedSincPolyDataFilter> smooth = vtkSmartPointer<vtkWindowedSincPolyDataFilter>::New();
    smooth->SetInputData( poly_data );
    smooth->SetPassBand( 0.1 );
    smooth->SetNumberOfIterations( 200 );
    smooth->FeatureEdgeSmoothingOn();
    smooth->NonManifoldSmoothingOn();
    smooth->Update();
    poly_data = smooth->GetOutput();
 */

/*
   vtkSmartPointer<vtkLoopSubdivisionFilter> subdivision = vtkSmartPointer<vtkLoopSubdivisionFilter>::New();
   subdivision->SetInputData( poly_data );
   subdivision->SetNumberOfSubdivisions( 1 );
   subdivision->Update();
   poly_data = subdivision->GetOutput();
 */

    // Make the triangle winding order consistent
    vtkSmartPointer<vtkPolyDataNormals> normals =
      vtkSmartPointer<vtkPolyDataNormals>::New();
    normals->SetInputData( poly_data );
    normals->ConsistencyOn();
    normals->SplittingOff();
    normals->Update();
    poly_data = normals->GetOutput();

/*
    vtkSmartPointer<vtkSmoothPolyDataFilter> smooth = vtkSmartPointer<vtkSmoothPolyDataFilter>::New();
    smooth->SetInputData( poly_data );
    smooth->SetNumberOfIterations( 200 );
    smooth->Update();
    poly_data = smooth->GetOutput();
 */

/*
    vtkSmartPointer<vtkWindowedSincPolyDataFilter> smooth = vtkSmartPointer<vtkWindowedSincPolyDataFilter>::New();
    smooth->SetInputData( poly_data );
    smooth->SetNumberOfIterations( 200 );
    smooth->SetPassBand(0.20);
    smooth->Update();
    poly_data = smooth->GetOutput();
 */

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

//-----------------------------------------------------------------------------
int Structure::get_id()
{
  return this->id_;
}

//-----------------------------------------------------------------------------
double Structure::get_volume()
{
  return 0;
  vtkSmartPointer<vtkPolyData> mesh = this->get_mesh();

  vtkSmartPointer<vtkMassProperties> mass_properties = vtkSmartPointer<vtkMassProperties>::New();

  mass_properties->SetInputData( mesh );
  mass_properties->Update();

  return mass_properties->GetVolume();
}

//-----------------------------------------------------------------------------
QString Structure::get_center_of_mass_string()
{
  return "foo";
  vtkSmartPointer<vtkPolyData> mesh = this->get_mesh();

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
vtkSmartPointer<vtkPolyData> Structure::get_mesh()
{
  if ( this->mesh_ )
  {
    return this->mesh_;
  }

  PointSampler ps( this );
  std::list<Weighted_point> points = ps.collect_spheres();

  std::cerr << "Generate union of spheres\n";
  Union_of_balls_3 union_of_balls( points.begin(), points.end() );
  Polyhedron P;
  CGAL::mesh_union_of_balls_3( union_of_balls, P );
  //CGAL::subdivide_union_of_balls_mesh_3(union_of_balls, P);

  double shrinkfactor = 0.5;
  //CGAL::make_skin_surface_mesh_3(P, points.begin(), points.end(), shrinkfactor);

/*
   typedef CGAL::Skin_surface_3<Traits> Skin_surface_3;
   Skin_surface_3 skin_surface(points.begin(), points.end(), shrinkfactor);
   CGAL::mesh_skin_surface_3(skin_surface, P);
   CGAL::subdivide_skin_surface_mesh_3(skin_surface, P);
 */

  std::ofstream out( "Z:\\shared\\balls.off" );

  out << P;
  out.close();

  std::cerr << "Convert resulting mesh\n";

  vtkSmartPointer<vtkPolyData> poly_data = vtkSmartPointer<vtkPolyData>::New();
  vtkSmartPointer<vtkPoints> vtk_pts = vtkSmartPointer<vtkPoints>::New();
  vtkSmartPointer<vtkCellArray> vtk_triangles = vtkSmartPointer<vtkCellArray>::New();

  typedef Polyhedron::Vertex_const_iterator VCI;
  typedef Polyhedron::Facet_const_iterator FCI;
  typedef Polyhedron::Halfedge_around_facet_const_circulator HFCC;
  typedef CGAL::Inverse_index<VCI> Index;

  int vcount = 0;
  for ( VCI vi = P.vertices_begin(); vi != P.vertices_end(); ++vi )
  {
    vtk_pts->InsertNextPoint( vi->point().x(), vi->point().y(), vi->point().z() );
    vcount++;
  }

  std::cerr << "inserted " << vcount << " vertices\n";

  Index index( P.vertices_begin(), P.vertices_end() );

  int fcount = 0;
  for ( FCI fi = P.facets_begin(); fi != P.facets_end(); ++fi )
  {
    fcount++;
    HFCC hc = fi->facet_begin();
    HFCC hc_end = hc;
    std::size_t n = circulator_size( hc );
    CGAL_assertion( n >= 3 );
    if ( n != 3 )
    {
      std::cerr << "Not triangular!\n";
    }
    vtkSmartPointer<vtkTriangle> triangle = vtkSmartPointer<vtkTriangle>::New();
    int c = 0;
    do
    {
      triangle->GetPointIds()->InsertId( c++, index[ VCI( hc->vertex() )] );
      ++hc;
    }
    while ( hc != hc_end );

    vtk_triangles->InsertNextCell( triangle );
  }

  std::cerr << "inserted " << fcount << " facets\n";

  poly_data->SetPoints( vtk_pts );
  poly_data->SetPolys( vtk_triangles );

  vtkSmartPointer<vtkSmoothPolyDataFilter> smooth;

  vtkSmartPointer<vtkFeatureEdges> features = vtkSmartPointer<vtkFeatureEdges>::New();
  features->SetInputData( poly_data );
  features->NonManifoldEdgesOn();
  features->BoundaryEdgesOff();
  features->FeatureEdgesOff();
  features->Update();

  vtkSmartPointer<vtkPolyData> nonmanifold = features->GetOutput();

  std::cerr << "Number of non-manifold points: " << nonmanifold->GetNumberOfPoints() << "\n";
  std::cerr << "Number of non-manifold cells: " << nonmanifold->GetNumberOfCells() << "\n";

    std::cerr << "QuadricDecimation\n";
    vtkSmartPointer<customQuadricDecimation> decimate = vtkSmartPointer<customQuadricDecimation>::New();
    decimate->SetInputData( poly_data );
    //decimate->SetTargetReduction(.99); //99% reduction (if there was 100 triangles, now there will be 1)
    decimate->SetTargetReduction( .98 ); //10% reduction (if there was 100 triangles, now there will be 90)
    decimate->Update();
    poly_data = decimate->GetOutput();

  features = vtkSmartPointer<vtkFeatureEdges>::New();
  features->SetInputData( poly_data );
  features->NonManifoldEdgesOn();
  features->BoundaryEdgesOff();
  features->FeatureEdgesOff();
  features->Update();

  nonmanifold = features->GetOutput();

  std::cerr << "Number of non-manifold points: " << nonmanifold->GetNumberOfPoints() << "\n";
  std::cerr << "Number of non-manifold cells: " << nonmanifold->GetNumberOfCells() << "\n";

/*
   std::cerr << "=====================Mesh Fixing====================\n";

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

   std::cerr << "=====================Mesh Fixing====================\n";
 */

/*
   std::cerr << "DecimatePro\n";
   vtkSmartPointer<vtkDecimatePro> decimate =
    vtkSmartPointer<vtkDecimatePro>::New();
   decimate->SetInputData(poly_data);
   //decimate->SetTargetReduction(.99); //99% reduction (if there was 100 triangles, now there will be 1)
   decimate->SetTargetReduction(.95); //10% reduction (if there was 100 triangles, now there will be 90)
   decimate->Update();
   poly_data = decimate->GetOutput();
 */

/*
   vtkSmartPointer< vtkTriangleFilter > triangle_filter = vtkSmartPointer< vtkTriangleFilter >::New();
   triangle_filter->SetInputData( poly_data );
   triangle_filter->Update();
   poly_data = triangle_filter->GetOutput();
 */



   vtkSmartPointer<vtkLoopSubdivisionFilter> subdivision = vtkSmartPointer<vtkLoopSubdivisionFilter>::New();
   subdivision->SetInputData( poly_data );
   subdivision->SetNumberOfSubdivisions( 2 );
   subdivision->Update();
   poly_data = subdivision->GetOutput();

   

/*
   std::cerr << "Sinc\n";
   vtkSmartPointer<vtkWindowedSincPolyDataFilter> sinc = vtkSmartPointer<vtkWindowedSincPolyDataFilter>::New();
   sinc->SetInputData( poly_data );
   sinc->SetNumberOfIterations(15);
   sinc->BoundarySmoothingOn();
   sinc->FeatureEdgeSmoothingOff();
   sinc->SetFeatureAngle(120.0);
   sinc->SetPassBand(.1);
   sinc->NormalizeCoordinatesOn();
   sinc->Update();
   poly_data = sinc->GetOutput();
 */

/*
   smooth->SetInputData( poly_data );
   smooth->SetNumberOfIterations( 100 );
   smooth->FeatureEdgeSmoothingOff();
   smooth->BoundarySmoothingOn();
   smooth->Update();
   poly_data = smooth->GetOutput();
 */

/*
   vtkSmartPointer<vtkCleanPolyData> clean = vtkSmartPointer<vtkCleanPolyData>::New();
   clean->SetInputData( poly_data );
   clean->SetTolerance( 0.00001 );
   clean->Update();
   poly_data = clean->GetOutput();


   vtkSmartPointer< vtkTriangleFilter > triangle_filter =
   vtkSmartPointer< vtkTriangleFilter >::New();
   triangle_filter->SetInputData( poly_data );
   triangle_filter->Update();
   poly_data = triangle_filter->GetOutput();
 */

/*
   std::cerr << "Linear subdivision\n";
   vtkSmartPointer<vtkLinearSubdivisionFilter> subdivision = vtkSmartPointer<vtkLinearSubdivisionFilter>::New();
   subdivision->SetInputData( poly_data );
   subdivision->SetNumberOfSubdivisions( 2 );
   subdivision->Update();
   poly_data = subdivision->GetOutput();
 */

/*
   std::cerr << "butterfly subdivision\n";
   vtkSmartPointer<vtkButterflySubdivisionFilter> subdivision = vtkSmartPointer<vtkButterflySubdivisionFilter>::New();
   subdivision->SetInputData( poly_data );
   subdivision->SetNumberOfSubdivisions( 2 );
   subdivision->Update();
   poly_data = subdivision->GetOutput();
 */

/*
   std::cerr << "Butterfly\n";
   vtkSmartPointer<vtkButterflySubdivisionFilter> butterfly = vtkSmartPointer<vtkButterflySubdivisionFilter>::New();
   butterfly->SetInputData(poly_data);
   butterfly->SetNumberOfSubdivisions(1);
   butterfly->Update();
   poly_data = butterfly->GetOutput();
 */

  /*
     smooth = vtkSmartPointer<vtkSmoothPolyDataFilter>::New();
     smooth->SetInputData( poly_data );
     smooth->SetNumberOfIterations( 100 );
     smooth->FeatureEdgeSmoothingOff();
     smooth->BoundarySmoothingOn();
     smooth->Update();
     poly_data = smooth->GetOutput();
   */

/*
   std::cerr << "Sinc\n";
   sinc = vtkSmartPointer<vtkWindowedSincPolyDataFilter>::New();
   sinc->SetInputData( poly_data );
   sinc->SetNumberOfIterations(15);
   sinc->BoundarySmoothingOn();
   sinc->FeatureEdgeSmoothingOff();
   sinc->SetFeatureAngle(120.0);
   sinc->SetPassBand(.5);
   sinc->NormalizeCoordinatesOn();
   sinc->Update();
   poly_data = sinc->GetOutput();
 */

/*
   std::cerr << "Normals\n";
   // Make the triangle winding order consistent
   vtkSmartPointer<vtkPolyDataNormals> normals =
    vtkSmartPointer<vtkPolyDataNormals>::New();
   normals->SetInputData( poly_data );
   normals->ConsistencyOn();
   normals->SplittingOff();
   normals->Update();
   poly_data = normals->GetOutput();
 */

  vtkSmartPointer<vtkSTLWriter> writer = vtkSmartPointer<vtkSTLWriter>::New();
  writer->SetFileName( "Z:\\shared\\file.stl" );
  writer->SetInputData( poly_data );
  writer->Write();

  this->mesh_ = poly_data;
  return this->mesh_;
}
