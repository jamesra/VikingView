#ifndef VIKING_DATA_STRUCTURE_H
#define VIKING_DATA_STRUCTURE_H

#include <map>

#include <QSharedPointer.h>

#include <vtkSmartPointer.h>



class vtkPolyData;

class Node
{
public:
  double x, y, z, radius;
  long id;
};

typedef std::map<long, Node> NodeMap;


class Link
{
public:
  long a, b;
};

//! Maintains data a structure (e.g. cell)
class Structure
{

public:
  ~Structure();


  static QSharedPointer<Structure> create_structure(int id, QString location_text, QString link_text);

  NodeMap get_node_map();

  QList<Link> get_links();

  vtkSmartPointer<vtkPolyData> get_mesh();

  double get_volume();

  QString get_center_of_mass_string();

  int get_id();

private:

  Structure(); // private

  int id_;
  NodeMap node_map_;

  QList<Link> links_;
  vtkSmartPointer<vtkPolyData> mesh_;

};


#endif /* VIKING_DATA_STRUCTURE_H */
