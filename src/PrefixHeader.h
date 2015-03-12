#include <iostream>

#include <QtGui>
#include <QtCore>
#include <QMainWindow>
#include <QActionGroup>
#include <QFileDialog>
#include <QVector>
#include <QSharedPointer>
#include <QString>

#include <vtkSmartPointer.h>
#include <vtkRenderer.h>
#include <vtkMarchingCubes.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkSphereSource.h>
#include <vtkGlyph3D.h>
#include <vtkProperty.h>
#include <vtkLookupTable.h>
#include <vtkUnsignedLongArray.h>
#include <vtkBooleanOperationPolyDataFilter.h>
#include <vtkAppendPolyData.h>
#include <vtkTubeFilter.h>

#include <vtkTriangle.h>
#include <vtkMath.h>

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Delaunay_triangulation_3.h>
#include <CGAL/Alpha_shape_3.h>
#include <CGAL/Union_of_balls_3.h>
#include <CGAL/mesh_union_of_balls_3.h>



#include <CGAL/IO/Polyhedron_iostream.h>
#include <CGAL/Inverse_index.h>
#include <CGAL/make_skin_surface_mesh_3.h>
