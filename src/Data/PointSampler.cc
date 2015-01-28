#include <Data/PointSampler.h>


class Node
{
public:
  double x,y,z,radius;


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


  std::map<int,Node> node_map;


  foreach( QVariant var, this->locations_ ) {
    Node n;
    QMap<QString, QVariant> item = var.toMap();
    n.x = item["VolumeX"].toDouble();
    n.y = item["VolumeY"].toDouble();
    n.z = item["Z"].toDouble();
    n.radius = item["Radius"].toDouble();

//    foreach( QString key, item.keys() ) {
//      std::cerr << key.toStdString() << " => " << item.value( key ).toString().toStdString() << '\n';
//    }
  }




  std::list<Point> points;

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

