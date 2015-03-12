#ifndef VIKING_DATA_POINTSAMPLER_H
#define VIKING_DATA_POINTSAMPLER_H

#include <QList>
#include <QVariant>

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
typedef K::Point_3 Point;


#include <CGAL/Union_of_balls_3.h>
#include <CGAL/mesh_union_of_balls_3.h>
typedef K::Point_3 Point_3;
typedef CGAL::Skin_surface_traits_3<K> Traits;
typedef CGAL::Union_of_balls_3<Traits> Union_of_balls_3;
typedef Union_of_balls_3::Weighted_point Weighted_point;
typedef Weighted_point::Point Bare_point;
typedef CGAL::Polyhedron_3<K> Polyhedron;
typedef Polyhedron::Facet_iterator Facet_iterator;
typedef Polyhedron::Halfedge_around_facet_circulator Halfedge_facet_circulator;
typedef Polyhedron::Point_iterator Point_iterator;
class Structure;

class PointSampler
{

public:
  PointSampler( Structure* structure );
  ~PointSampler();

  std::list<Point> sample_points();

  std::list<Weighted_point> collect_spheres();

private:


  static void sample_sphere(double radius, double x, double y, double z, std::list<Point> &points);

  Structure* structure_;
};

#endif /* VIKING_DATA_POINTSAMPLER_H */
