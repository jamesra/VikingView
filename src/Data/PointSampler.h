#ifndef VIKING_DATA_POINTSAMPLER_H
#define VIKING_DATA_POINTSAMPLER_H

#include <QList>
#include <QVariant>

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
typedef K::Point_3 Point;

class Structure;

class PointSampler
{

public:
  PointSampler(QSharedPointer<Structure> structure);
  ~PointSampler();

  std::list<Point> sample_points();

private:

  QSharedPointer<Structure> structure_;



};


#endif /* VIKING_DATA_POINTSAMPLER_H */
