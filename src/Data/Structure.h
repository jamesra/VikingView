#ifndef VIKING_DATA_STRUCTURE_H
#define VIKING_DATA_STRUCTURE_H


#include <QSharedPointer.h>
#include <QHash.h>
#include <QColor.h>

#include <vtkSmartPointer.h>
#include <vtkBoundingBox.h>
#include <Data/Scale.h>
#include <Data/ColorMapper.h>

class vtkPolyData;
class vtkAppendPolyData;

class Node
{
public:
  double x, y, z, radius;
  long id;
  long parent_id;
  QList<long> linked_nodes;
  long graph_id;
  bool visited;

  inline bool IsEndpoint() { return linked_nodes.size() < 2; }
  inline bool IsBranch() { return linked_nodes.size() > 2; }
};

typedef QHash<long, QSharedPointer<Node> > NodeMap;

class Link
{
public:
  long a, b;
};


class Structure;

QList<QSharedPointer<Structure>> GatherStructures(QList<QSharedPointer<Structure>> root_structures);

typedef QHash<long, QSharedPointer<Structure> > StructureHash;

//! Maintains data a structure (e.g. cell)
class Structure
{

public: 
  Structure(QSharedPointer<QColor> color);
  ~Structure();

  static QSharedPointer<StructureHash> create_structures( QList<QVariant> structure_list,
                                                          QList<QVariant> location_list, 
														  QList<QVariant> link_list,
														  QSharedPointer<ScaleObject> scale,
														  ColorMapper cmap, 
														  bool reverse_Z);

  NodeMap get_node_map();

  QList<Link> get_links();

  vtkSmartPointer<vtkPolyData> get_mesh_old();
  vtkSmartPointer<vtkPolyData> get_mesh_alpha();
  // vtkSmartPointer<vtkPolyData> get_mesh_union();
  vtkSmartPointer<vtkPolyData> get_mesh_parts();
  vtkSmartPointer<vtkPolyData> get_mesh_tubes();
  vtkSmartPointer<vtkPolyData> get_mesh();

  QSharedPointer<ScaleObject> scale;

  double get_volume();

  QString get_center_of_mass_string();

  int get_id();
  int get_type();
  int get_parent_id();
  QString get_label() { return label_; }

  bool has_parent() { return parent_id_ > 0; }

  void set_color( QColor color );

  QColor get_color();

  QSharedPointer<vtkBoundingBox> get_bbox();

  vtkSmartPointer<vtkPolyData> recopy_mesh( vtkSmartPointer<vtkPolyData> mesh );

  StructureHash structures;

private:

  Structure(); // private

  static QSharedPointer<Structure>  structure_from_json(QVariant var);

  void add_polydata( QSharedPointer<Node> n, int from, vtkSmartPointer<vtkAppendPolyData> append, 
    QList<int> current_line );

  static double distance( const QSharedPointer<Node> &n1, const QSharedPointer<Node> &n2 );

  void connect_subgraphs();

  void remove_node(long id);
    
  void cull_outliers();

  void cull_overlapping();

  void link_report();

  int id_;
  int type_;
  int parent_id_;
  NodeMap node_map_;

  QList<Link> links_;
  vtkSmartPointer<vtkPolyData> mesh_;
  
  QString label_;
  QColor color_;

  int num_tubes_;

//  float color_[3];
};

#endif /* VIKING_DATA_STRUCTURE_H */
