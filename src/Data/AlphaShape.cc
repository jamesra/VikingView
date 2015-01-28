#include <Data/AlphaShape.h>

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Delaunay_triangulation_3.h>
#include <CGAL/Alpha_shape_3.h>
#include <fstream>
#include <list>
#include <cassert>
typedef CGAL::Exact_predicates_inexact_constructions_kernel Gt;
typedef CGAL::Alpha_shape_vertex_base_3<Gt> Vb;
typedef CGAL::Alpha_shape_cell_base_3<Gt> Fb;
typedef CGAL::Triangulation_data_structure_3<Vb, Fb> Tds;
//typedef CGAL::Delaunay_triangulation_3<Gt,Tds,CGAL::Fast_location> Triangulation_3;
typedef CGAL::Delaunay_triangulation_3<Gt, Tds> Triangulation_3;
typedef CGAL::Alpha_shape_3<Triangulation_3> Alpha_shape_3;
typedef Gt::Point_3 Point;
typedef Alpha_shape_3::Alpha_iterator Alpha_iterator;

int cgal_main( int argc, char** argv )
{

  std::list<Point> lp;
  //read input
  std::ifstream is( argv[1] );
  int n;
  is >> n;
  //std::cout << "Reading " << n << " points " << std::endl;
  Point p;
  for (; n > 0; n-- )
  {
    is >> p;
    lp.push_back( p );
  }
  // compute alpha shape
  Alpha_shape_3 as( lp.begin(), lp.end() );
  std::cout << "Alpha shape computed in REGULARIZED mode by default"
            << std::endl;

  float alpha = -1;
  if ( argc == 4 )
  {
    alpha = atof( argv[3] );
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

  /// collect all regular facets
  std::vector<Alpha_shape_3::Facet> facets;
  as.get_alpha_shape_facets( std::back_inserter( facets ), Alpha_shape_3::REGULAR );

  std::stringstream pts;
  std::stringstream ind;

  std::size_t nbf = facets.size();
  for ( std::size_t i = 0; i < nbf; ++i )
  {

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
    }
  }

  std::ofstream os( argv[2] );

  os << "OFF " << 3 * nbf << " " << nbf << " 0\n";
  os << pts.str();
  os << ind.str();

  return 0;
}

//-----------------------------------------------------------------------------
AlphaShape::AlphaShape()
{

}

//-----------------------------------------------------------------------------
AlphaShape::~AlphaShape()
{

}

//-----------------------------------------------------------------------------
void AlphaShape::set_locations(QList<QVariant> locations)
{
  this->locations_ = locations;
}

//-----------------------------------------------------------------------------
vtkPolyData * AlphaShape::get_mesh()
{

  return 0;
}
