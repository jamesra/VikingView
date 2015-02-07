#include <QSharedPointer.h>

#include <Data/PointSampler.h>
#include <Data/Structure.h>

#include <vtkMath.h>

//-----------------------------------------------------------------------------
PointSampler::PointSampler( Structure* structure )
{
  this->structure_ = structure;
}

//-----------------------------------------------------------------------------
PointSampler::~PointSampler()
{}

//-----------------------------------------------------------------------------
std::list<Point> PointSampler::sample_points()
{

  /*
     int num_radii = 3;
     int num_points = 15;
     int num_pts_circle = 10;
     int num_pts_line = 10;
   */

  int num_radii = 1;
  int num_pts_circle = 50;
  int num_pts_line = 20;
  //int num_pts_circle = 3;
  //int num_pts_line = 2;

  NodeMap node_map = this->structure_->get_node_map();

  std::list<Point> points;

  // spheres
  for ( NodeMap::iterator it = node_map.begin(); it != node_map.end(); ++it )
  {

    Node n = it->second;

    if ( n.linked_nodes.size() != 1 )
    {
      continue;
    }

    PointSampler::sample_sphere( n.radius, n.x, n.y, n.z, points );
  }

  // cylinders

  //std::ofstream out;
  //out.open( "C:\\Users\\amorris\\points.out" );

  foreach( Link link, this->structure_->get_links() ) {

    if ( node_map.find( link.a ) == node_map.end() || node_map.find( link.b ) == node_map.end() )
    {
      continue;
    }

    Node n1 = node_map[link.a];
    Node n2 = node_map[link.b];

    double p1[3], p2[3];

    p1[0] = n1.x;
    p1[1] = n1.y;
    p1[2] = n1.z;
    p2[0] = n2.x;
    p2[1] = n2.y;
    p2[2] = n2.z;

    double vec[3];
    vec[0] = p1[0] - p2[0];
    vec[1] = p1[1] - p2[1];
    vec[2] = p1[2] - p2[2];

    double squared_distance = vtkMath::Distance2BetweenPoints( p1, p2 );
    double distance = sqrt( squared_distance );

    int this_num_points = ( num_pts_line * distance );
    if ( this_num_points <= 3 )
    {
      this_num_points = 3;
    }

    vtkMath::Normalize( vec );

    double other[3];
    other[0] = 0.25;
    other[1] = 0.25;
    other[2] = 0.15;

    double u[3];
    vtkMath::Cross( vec, other, u );
    vtkMath::Normalize( u );

    double v[3];
    vtkMath::Cross( vec, u, v );

    double this_num_circle = num_pts_circle;
    double this_num_radii = num_radii;

    double avg_overall_radius = ( n1.radius + n2.radius ) / 2.0;

    this_num_radii = 3 * avg_overall_radius;
    if ( this_num_radii < 1 )
    {
      this_num_radii = 1;
    }
    else
    {
      //std::cerr << "rad = " << this_num_radii << "\n";
    }

    this_num_radii = (int)this_num_radii;


    for ( double r = 1; r <= this_num_radii; r++ )
    {
      double n1_radius = n1.radius * ( r / this_num_radii );
      double n2_radius = n2.radius * ( r / this_num_radii );

      double avg_radius = ( n1_radius + n2_radius ) / 2.0;
      this_num_circle = num_pts_circle * avg_radius;
      if ( this_num_circle < 3 )
      {
        this_num_circle = 3;
      }

      for ( double i = 0; i < this_num_circle; i++ )
      {
        double theta = (double)i * 2 * M_PI / this_num_circle;

        double circ1_x = n1.x + n1_radius * cos( theta ) * u[0] + n1_radius * sin( theta ) * v[0];
        double circ1_y = n1.y + n1_radius * cos( theta ) * u[1] + n1_radius * sin( theta ) * v[1];
        double circ1_z = n1.z + n1_radius * cos( theta ) * u[2] + n1_radius * sin( theta ) * v[2];

        double circ2_x = n2.x + n2_radius * cos( theta ) * u[0] + n2_radius * sin( theta ) * v[0];
        double circ2_y = n2.y + n2_radius * cos( theta ) * u[1] + n2_radius * sin( theta ) * v[1];
        double circ2_z = n2.z + n2_radius * cos( theta ) * u[2] + n2_radius * sin( theta ) * v[2];

        for ( double j = 0; j < this_num_points; j++ )
        {
          double ratio = j / ( (double)this_num_points - 1 );
          double inv_ratio = 1 - ratio;

          double this_x = circ1_x * ratio + circ2_x * inv_ratio;
          double this_y = circ1_y * ratio + circ2_y * inv_ratio;
          double this_z = circ1_z * ratio + circ2_z * inv_ratio;

          Point p( this_x, this_y, this_z );
          points.push_back( p );

          if ( this_x == 0 )
          {
            std::cerr << "oh no, break!\n";
          }
        }
      }
    }
  }

  std::ofstream out;
  out.open( "C:\\Users\\amorris\\points.asc" );
  for ( std::list<Point>::iterator it = points.begin(); it != points.end(); ++it )
  {
    out << ( *it ).x() << " " << ( *it ).y() << " " << ( *it ).z() << "\n";
  }

  out.close();

  std::cerr << "Sampled " << points.size() << " points\n";

  return points;
}

//-----------------------------------------------------------------------------
void PointSampler::sample_sphere( double radius, double ox, double oy, double oz, std::list<Point> &points )
{

  int rnd = 1;

  int samples = 500 * radius;

  if ( samples < 10 )
  {
    samples = 10;
  }

  double offset = 2.0 / (double)samples;

  double increment = M_PI * ( 3.0 - sqrt( 5.0 ) );

  for ( int i = 0; i < samples; i++ )
  {
    double y = ( ( i * offset ) - 1 ) + ( offset / 2 );
    double r = sqrt( 1 - pow( y, 2 ) );

    double phi = ( ( i + rnd ) % samples ) * increment;

    double x = cos( phi ) * r;
    double z = sin( phi ) * r;

    Point p( ox + ( x * radius ), oy + ( y * radius ), oz + ( z * radius ) );
    points.push_back( p );
  }
}
