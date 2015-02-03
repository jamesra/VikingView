#include <Data/Structure.h>
#include <Data/Json.h>

QSharedPointer<Structure> Structure::create_structure( QString location_text, QString link_text )
{
  QSharedPointer<Structure> structure = QSharedPointer<Structure>( new Structure() );

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

Structure::Structure()
{}

Structure::~Structure()
{}

NodeMap Structure::get_node_map()
{
  return this->node_map_;
}
