#include <Data/AlphaShape.h>

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Delaunay_triangulation_3.h>
#include <CGAL/Alpha_shape_3.h>
#include <fstream>
#include <list>
#include <cassert>

#include <vtkSmartPointer.h>
#include <vtkCellArray.h>
#include <vtkTriangle.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>

#include <vtkPolyDataWriter.h>

//#include <vtkXMLPolyDataWriter.h>

#include <vtkDataSetWriter.h>

typedef CGAL::Exact_predicates_inexact_constructions_kernel Gt;
typedef CGAL::Alpha_shape_vertex_base_3<Gt> Vb;
typedef CGAL::Alpha_shape_cell_base_3<Gt> Fb;
typedef CGAL::Triangulation_data_structure_3<Vb, Fb> Tds;
//typedef CGAL::Delaunay_triangulation_3<Gt,Tds,CGAL::Fast_location> Triangulation_3;
typedef CGAL::Delaunay_triangulation_3<Gt, Tds> Triangulation_3;
typedef CGAL::Alpha_shape_3<Triangulation_3> Alpha_shape_3;
typedef Gt::Point_3 Point;
typedef Alpha_shape_3::Alpha_iterator Alpha_iterator;

//-----------------------------------------------------------------------------
AlphaShape::AlphaShape()
{}

//-----------------------------------------------------------------------------
AlphaShape::~AlphaShape()
{}

//-----------------------------------------------------------------------------
vtkSmartPointer<vtkPolyData> AlphaShape::get_mesh()
{
  // compute alpha shape
  Alpha_shape_3 as( this->points_.begin(), this->points_.end() );
  std::cout << "Alpha shape computed in REGULARIZED mode by default"
            << std::endl;

  float alpha = -1;
  if ( alpha != -1 )
  {
    std::cout << "Using Alpha = " << alpha << "\n";
    as.set_alpha( alpha );
  }
  else
  {
    // find optimal alpha value
    Alpha_iterator opt = as.find_optimal_alpha( 1 );
    std::cout << "Optimal alpha value to get one connected component is "
              << *opt << std::endl;
    as.set_alpha( *opt );

    assert( as.number_of_solid_components() == 1 );
  }

  vtkSmartPointer<vtkPolyData> poly_data = vtkSmartPointer<vtkPolyData>::New();

  vtkSmartPointer<vtkPoints> vtk_pts = vtkSmartPointer<vtkPoints>::New();
  vtkSmartPointer<vtkCellArray> vtk_triangles = vtkSmartPointer<vtkCellArray>::New();

  /// collect all regular facets
  std::vector<Alpha_shape_3::Facet> facets;
  as.get_alpha_shape_facets( std::back_inserter( facets ), Alpha_shape_3::REGULAR );

  std::stringstream pts;
  std::stringstream ind;

  std::map<Alpha_shape_3::Vertex_handle, int> vertex_map;

  std::size_t nbf = facets.size();
  int vertex_index = 0;

  for ( std::size_t i = 0; i < nbf; ++i )
  {

    Alpha_shape_3::Facet f = facets[i];

    //if ( as.classify( facets[i].first )==Alpha_shape_3::EXTERIOR )
    {

      //To have a consistent orientation of the facet, always consider an exterior cell
      if ( as.classify( facets[i].first ) != Alpha_shape_3::EXTERIOR )
      {
        facets[i] = as.mirror_facet( facets[i] );
      }
      CGAL_assertion( as.classify( facets[i].first ) == Alpha_shape_3::EXTERIOR );

      int indices[3] = {
        ( facets[i].second + 1 ) % 4,
        ( facets[i].second + 2 ) % 4,
        ( facets[i].second + 3 ) % 4,
      };

      /// according to the encoding of vertex indices, this is needed to get
      /// a consistent orienation
      if ( facets[i].second % 2 == 0 ) { std::swap( indices[0], indices[1] ); }

      pts <<
        facets[i].first->vertex( indices[0] )->point() << "\n" <<
        facets[i].first->vertex( indices[1] )->point() << "\n" <<
        facets[i].first->vertex( indices[2] )->point() << "\n";

      ind << "3 " << 3 * i << " " << 3 * i + 1 << " " << 3 * i + 2 << "\n";

      for ( int index = 0; index < 3; index++ )

      {
        if ( vertex_map.find( facets[i].first->vertex( indices[index] ) ) == vertex_map.end() )
        {
          // not found
          vertex_map[facets[i].first->vertex( indices[index] ) ] = vertex_index++;

          Point p = facets[i].first->vertex( indices[index] )->point();
          vtk_pts->InsertNextPoint( p.x(), p.y(), p.z() );
        }
      }

      // Create a triangle
      vtkSmartPointer<vtkTriangle> triangle =
        vtkSmartPointer<vtkTriangle>::New();
      triangle->GetPointIds()->SetId( 0, vertex_map[facets[i].first->vertex( indices[0] ) ] );
      triangle->GetPointIds()->SetId( 1, vertex_map[facets[i].first->vertex( indices[1] ) ] );
      triangle->GetPointIds()->SetId( 2, vertex_map[facets[i].first->vertex( indices[2] ) ] );

      // Add the triangle to a cell array
      vtk_triangles->InsertNextCell( triangle );
    }
  }
/*
   std::ofstream os("C:\\Users\\amorris\\file.OFF");

   os << "OFF "<< 3*nbf << " " << nbf << " 0\n";
   os << pts.str();
   os << ind.str();
 */

  std::cerr << "Created polydata with " << vertex_index << " points, " << vtk_triangles->GetNumberOfCells() << " triangles\n";

  vtkSmartPointer<vtkPolyData> polydata =
    vtkSmartPointer<vtkPolyData>::New();
  polydata->SetPoints( vtk_pts );
  polydata->SetPolys( vtk_triangles );

  vtkSmartPointer<vtkPolyDataWriter> writer4 = vtkSmartPointer<vtkPolyDataWriter>::New();
  writer4->SetFileName( "C:\\Users\\amorris\\file.vtk" );
  writer4->SetInputData( polydata );
//writer4->SetInputConnection( geometry->GetOutputPort() );
  writer4->Write();

  return polydata;
}

//-----------------------------------------------------------------------------
void AlphaShape::set_points( std::list<Point> points )
{
  this->points_ = points;
}
