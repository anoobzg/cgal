// Copyright (c) 2007-09  INRIA Sophia-Antipolis (France).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org); you may redistribute it under
// the terms of the Q Public License version 1.0.
// See the file LICENSE.QPL distributed with CGAL.
//
// Licensees holding a valid commercial license may use this file in
// accordance with the commercial license agreement provided with the software.
//
// This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
// WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
//
// $URL: svn+ssh://scm.gforge.inria.fr/svn/cgal/trunk/Point_set_processing_3/include/CGAL/improved_laplacian_smooth_point_set.h $
// $Id: improved_laplacian_smooth_point_set.h 48866 2009-04-22 15:06:24Z lsaboret $
//
// Author(s) : Pierre Alliez and Marc Pouget

#ifndef CGAL_JET_SMOOTH_POINT_SET_H
#define CGAL_JET_SMOOTH_POINT_SET_H

#include <CGAL/Search_traits_3.h>
#include <CGAL/Orthogonal_k_neighbor_search.h>
#include <CGAL/Monge_via_jet_fitting.h>

#include <iterator>
#include <list>

CGAL_BEGIN_NAMESPACE


// ----------------------------------------------------------------------------
// Private section
// ----------------------------------------------------------------------------
namespace CGALi {


/// Smooth one point position using jet fitting on the k
/// nearest neighbors and reprojection onto the jet.
///
/// @commentheading Precondition: k >= 2.
///
/// @commentheading Template Parameters:
/// @param Kernel Geometric traits class.
/// @param Tree KD-tree.
///
/// @return computed point
template <typename Kernel,
          typename Tree>
typename Kernel::Point_3
jet_smooth_point_set(const typename Kernel::Point_3& query, ///< 3D point to project
                     Tree& tree, ///< KD-tree
                     const unsigned int k,
                     const unsigned int degree_fitting,
                     const unsigned int degree_monge)
{
  // basic geometric types
  typedef typename Kernel::Point_3 Point_3;

  // types for K nearest neighbors search
  typedef typename CGAL::Search_traits_3<Kernel> Tree_traits;
  typedef typename CGAL::Orthogonal_k_neighbor_search<Tree_traits> Neighbor_search;
  typedef typename Neighbor_search::iterator Search_iterator;

  // types for jet fitting
  typedef typename CGAL::Monge_via_jet_fitting<Kernel> Monge_jet_fitting;
  typedef typename Monge_jet_fitting::Monge_form Monge_form;

  // Gather set of (k+1) neighboring points.
  // Performs k + 1 queries (if unique the query point is
  // output first). Search may be aborted when k is greater
  // than number of input points.
  std::vector<Point_3> points; points.reserve(k+1);
  Neighbor_search search(tree,query,k+1);
  Search_iterator search_iterator = search.begin();
  unsigned int i;
  for(i=0;i<(k+1);i++)
  {
    if(search_iterator == search.end())
      break; // premature ending
    points.push_back(search_iterator->first);
    search_iterator++;
  }
  CGAL_precondition(points.size() >= 1);

  // performs jet fitting
  Monge_jet_fitting monge_fit;
  Monge_form monge_form = monge_fit(points.begin(), points.end(),
                                    degree_fitting, degree_monge);

  // output projection of query point onto the jet
  return monge_form.origin();
}


} /* namespace CGALi */


// ----------------------------------------------------------------------------
// Public section
// ----------------------------------------------------------------------------


/// Smooth a point set using jet fitting on the k
/// nearest neighbors and reprojection onto the jet.
///
/// Warning: as this method relocates the points, it
/// should not be called on containers sorted w.r.t. point locations.
///
/// @commentheading Precondition: k >= 2.
///
/// @commentheading Template Parameters:
/// @param ForwardIterator value_type must be convertible to Point_3<Kernel>.
/// @param Kernel Geometric traits class. It can be omitted and deduced automatically from the iterator type.

// This variant requires the kernel.
template <typename ForwardIterator,
          typename Kernel>
void
jet_smooth_point_set(ForwardIterator first,     ///< iterator over the first input/output point.
                     ForwardIterator beyond,    ///< past-the-end iterator.
                     unsigned int k,            ///< number of neighbors.
                     const Kernel& kernel,      ///< geometric traits.
                     const unsigned int degree_fitting = 2,
                     const unsigned int degree_monge = 2)
{
  // Point_3 types
  typedef typename std::iterator_traits<ForwardIterator>::value_type Input_point_3;
  typedef typename Kernel::Point_3 Point_3;

  // types for K nearest neighbors search structure
  typedef typename CGAL::Search_traits_3<Kernel> Tree_traits;
  typedef typename CGAL::Orthogonal_k_neighbor_search<Tree_traits> Neighbor_search;
  typedef typename Neighbor_search::Tree Tree;
  typedef typename Neighbor_search::iterator Search_iterator;

  // precondition: at least one element in the container.
  // to fix: should have at least three distinct points
  // but this is costly to check
  CGAL_precondition(first != beyond);

  // precondition: at least 2 nearest neighbors
  CGAL_precondition(k >= 2);

  // instanciate a KD-tree search
  Tree tree(first,beyond);

  // Iterate over input points and mutate them.
  // Note: the cast to (Point_3&) ensures compatibility with classes derived from Point_3.
  ForwardIterator it;
  for(it = first; it != beyond; it++)
    (Point_3&)(*it) = CGALi::jet_smooth_point_set<Kernel>(*it,tree,k,degree_fitting,degree_monge);
}

/// @cond SKIP_IN_MANUAL
// This variant deduces the kernel from iterator type.
template <typename ForwardIterator>
void
jet_smooth_point_set(ForwardIterator first, ///< iterator over the first input/output point
                     ForwardIterator beyond, ///< past-the-end iterator
                     unsigned int k, ///< number of neighbors
                     const unsigned int degree_fitting = 2,
                     const unsigned int degree_monge = 2)
{
  typedef typename std::iterator_traits<ForwardIterator>::value_type Input_point_3;
  typedef typename Kernel_traits<Input_point_3>::Kernel Kernel;
  jet_smooth_point_set(first,beyond,k,Kernel(),degree_fitting,degree_monge);
}
/// @endcond


CGAL_END_NAMESPACE

#endif // CGAL_JET_SMOOTH_POINT_SET_H

