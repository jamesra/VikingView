#ifndef VIKING_DATA_POINTSAMPLER_H
#define VIKING_DATA_POINTSAMPLER_H

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
typedef K::Point_3 Point;


class PointSampler
{

public:
  PointSampler();
  ~PointSampler();

  void set_locations(QList<QVariant> locations);
  //void set_links(QString links_json);

  std::list<Point> sample_points();

private:

  QList<QVariant> locations_;


};


#endif /* VIKING_DATA_POINTSAMPLER_H */
