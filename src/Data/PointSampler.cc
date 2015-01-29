#include <Data/PointSampler.h>

//#define M_PI           3.14159265358979323846  /* pi */

class Node
{
public:
  double x,y,z,radius;
  long id;


};


PointSampler::PointSampler()
{

}

PointSampler::~PointSampler()
{

}

void PointSampler::set_locations(QList<QVariant> locations)
{
  this->locations_ = locations;
}

std::list<Point> PointSampler::sample_points()
{


  typedef std::map<long, Node> MapType;
  MapType node_map;

  // construct nodes
  foreach( QVariant var, this->locations_ ) {
    Node n;
    QMap<QString, QVariant> item = var.toMap();
    n.x = item["VolumeX"].toDouble();
    n.y = item["VolumeY"].toDouble();
    n.z = item["Z"].toDouble();
    n.radius = item["Radius"].toDouble();
    n.id = item["ID"].toLongLong();
    node_map[n.id] = n;
  }

  // links
  // ....

  std::list<Point> points;


  int num_radii = 3;
  int num_points = 10;




  for(MapType::iterator it = node_map.begin(); it != node_map.end(); ++it) {
    // iterator->first = key
    // iterator->second = value
    // Repeat if you also want to iterate through the second map.
   
    Node n = it->second;

    for (int r=0; r<num_radii; r++)
    {
      float radius = n.radius * (float)r / (float)num_radii;

      for (int i = 0; i<num_points; i++)
      {
        float theta = (float)i * 2 * M_PI / (float)num_points;

        for (int j = 0; j<num_points; i++)
        {

          float ratio = 2 * (float)j / ((float)num_points - 1);

          float u = -radius + (ratio * radius);

          float radius_squared = radius * radius;
          float u_squared = u * u;

          float this_x = n.x + sqrt(radius_squared - u_squared) * cos(theta);
          float this_y = n.y + sqrt(radius_squared - u_squared) * sin(theta);
          float this_z = n.z + u;

/*
#        puts "#{i} #{j}: #{ratio}"
            p = Point.new
            p.x = this_x
            p.y = this_y
            p.z = this_z
            points.push(p)
*/

        }
      }

    }



  }



  int count = 0;
  foreach( QVariant var, this->locations_ ) {
    count++;
    if ( count < 2 )
    {
      QMap<QString, QVariant> item = var.toMap();
      foreach( QString key, item.keys() ) {
        std::cerr << key.toStdString() << " => " << item.value( key ).toString().toStdString() << '\n';
      }
    }
  }




  return points;
}

