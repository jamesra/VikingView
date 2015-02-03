#include <Data/PointSampler.h>
#include <Data/Structure.h>

//#define M_PI           3.14159265358979323846  /* pi */

PointSampler::PointSampler(QSharedPointer<Structure> structure)
{
  this->structure_ = structure;
}

PointSampler::~PointSampler()
{}

std::list<Point> PointSampler::sample_points()
{


  NodeMap node_map = this->structure_->get_node_map();

  // links
  // ....

  std::list<Point> points;

  int num_radii = 2;
  int num_points = 5;

  for ( NodeMap::iterator it = node_map.begin(); it != node_map.end(); ++it )
  {
   
    Node n = it->second;

    for ( int r = 0; r < num_radii; r++ )
    {
      float radius = n.radius * (float)r / (float)num_radii;

      for ( int i = 0; i < num_points; i++ )
      {
        float theta = (float)i * 2 * M_PI / (float)num_points;

        for ( int j = 0; j < num_points; j++ )
        {

          float ratio = 2 * (float)j / ( (float)num_points - 1 );

          float u = -radius + ( ratio * radius );

          float radius_squared = radius * radius;
          float u_squared = u * u;

          float this_x = n.x + sqrt( radius_squared - u_squared ) * cos( theta );
          float this_y = n.y + sqrt( radius_squared - u_squared ) * sin( theta );
          float this_z = n.z + u;

          Point p( this_x, this_y, this_z );
          points.push_back( p );
        }
      }
    }
  }

  std::cerr << "Sampled " << points.size() << " points\n";

  return points;
}
