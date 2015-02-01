#include <Data/PointSampler.h>

//#define M_PI           3.14159265358979323846  /* pi */

class Node
{
public:
  double x, y, z, radius;
  long id;
};

PointSampler::PointSampler()
{}

PointSampler::~PointSampler()
{}

void PointSampler::set_locations( QList<QVariant> locations )
{
  this->locations_ = locations;
}

std::list<Point> PointSampler::sample_points()
{

  typedef std::map<long, Node> MapType;
  MapType node_map;

  float units_per_pixel = 2.18 / 1000.0;
  float units_per_section = 90.0 / 1000.0;

  // construct nodes
  foreach( QVariant var, this->locations_ ) {
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

    node_map[n.id] = n;

  }

  // links
  // ....

  std::list<Point> points;

  int num_radii = 1;
  int num_points = 3;

  for ( MapType::iterator it = node_map.begin(); it != node_map.end(); ++it )
  {
    // iterator->first = key
    // iterator->second = value
    // Repeat if you also want to iterate through the second map.

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

  return points;
}
