#ifndef VIKING_DATA_STRUCTURE_H
#define VIKING_DATA_STRUCTURE_H



class Node
{
public:
  double x, y, z, radius;
  long id;
};

typedef std::map<long, Node> NodeMap;


//! Maintains data a structure (e.g. cell)
class Structure
{

public:
  ~Structure();


  static QSharedPointer<Structure> create_structure(QString location_text, QString link_text);

  NodeMap get_node_map();

private:
  Structure(); // private

  NodeMap node_map_;

};


#endif /* VIKING_DATA_STRUCTURE_H */
