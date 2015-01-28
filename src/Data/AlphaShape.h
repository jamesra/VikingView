#ifndef VIKING_DATA_ALPHASHAPE_H
#define VIKING_DATA_ALPHASHAPE_H

int cgal_main(int argc, char **argv);


class AlphaShape
{

public:
  AlphaShape();
  ~AlphaShape();

  void set_locations(QList<QVariant> locations);
  //void set_links(QString links_json);

  vtkPolyData *get_mesh();

private:

  QList<QVariant> locations_;


};


#endif /* VIKING_DATA_ALPHASHAPE_H */
