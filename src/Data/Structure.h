#ifndef VIKING_DATA_STRUCTURE_H
#define VIKING_DATA_STRUCTURE_H

#include <map>

#include <QSharedPointer.h>
#include <QColor.h>

#include <vtkSmartPointer.h>

class vtkPolyData;

class Node
{
public:
  double x, y, z, radius;
  long id;
  QList<int> linked_nodes;
  long graph_id;
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

public: ~Structure();

  static QSharedPointer<Structure> create_structure( int id, QList<QVariant> location_list, QList<QVariant> link_list );

  NodeMap get_node_map();

  QList<Link> get_links();

  vtkSmartPointer<vtkPolyData> get_mesh_old();
  vtkSmartPointer<vtkPolyData> get_mesh_alpha();

  vtkSmartPointer<vtkPolyData> get_mesh_union();

  vtkSmartPointer<vtkPolyData> get_mesh_parts();

  double get_volume();

  QString get_center_of_mass_string();

  int get_id();

  void set_color(QColor color);

  QColor get_color();

  vtkSmartPointer<vtkPolyData> recopy_mesh(vtkSmartPointer<vtkPolyData> mesh);

private:

  Structure(); // private

  int id_;
  NodeMap node_map_;

  QList<Link> links_;
  vtkSmartPointer<vtkPolyData> mesh_;

  QColor color_;

//  float color_[3];

};

#endif /* VIKING_DATA_STRUCTURE_H */
