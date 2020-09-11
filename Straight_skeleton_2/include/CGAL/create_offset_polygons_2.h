// Copyright (c) 2006-2008 Fernando Luis Cacciola Carballal. All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//

// $URL$
// $Id$
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
// Author(s)     : Fernando Cacciola <fernando_cacciola@ciudad.com.ar>
//
#ifndef CGAL_CREATE_OFFSET_POLYGONS_2_H
#define CGAL_CREATE_OFFSET_POLYGONS_2_H

#include <CGAL/license/Straight_skeleton_2.h>

#include <CGAL/create_straight_skeleton_2.h>
#include <CGAL/Polygon_offset_builder_2.h>
#include <CGAL/Straight_skeleton_converter_2.h>
#include <CGAL/compute_outer_frame_margin.h>
#include <CGAL/Polygon_with_holes_2.h>

namespace CGAL {

namespace CGAL_SS_i {

template<class U, class V> struct Is_same_type { typedef Tag_false type ; } ;

template<class U> struct Is_same_type<U,U> { typedef Tag_true type ; } ;


template<class FT, class PointIterator, class HoleIterator, class K>
boost::shared_ptr< Straight_skeleton_2<K> >
create_partial_interior_straight_skeleton_2 ( FT const&     aMaxTime
                                            , PointIterator aOuterContour_VerticesBegin
                                            , PointIterator aOuterContour_VerticesEnd
                                            , HoleIterator  aHolesBegin
                                            , HoleIterator  aHolesEnd
                                            , K const& // aka 'SK'
                                            )
{
  CGAL_precondition( aMaxTime > static_cast<FT>(0) ) ;

  typedef Straight_skeleton_2<K> Ss ;
  typedef Straight_skeleton_builder_traits_2<K> SsBuilderTraits;
  typedef Straight_skeleton_builder_2<SsBuilderTraits,Ss> SsBuilder;

  typedef typename K::FT KFT ;

  typedef typename std::iterator_traits<PointIterator>::value_type InputPoint ;
  typedef typename Kernel_traits<InputPoint>::Kernel InputKernel ;

  Cartesian_converter<InputKernel, K> Converter ;

  KFT kMaxTime = aMaxTime;
  boost::optional<KFT> lMaxTime(kMaxTime) ;

  SsBuilder ssb( lMaxTime ) ;

  ssb.enter_contour( aOuterContour_VerticesBegin, aOuterContour_VerticesEnd, Converter ) ;

  for ( HoleIterator hi = aHolesBegin ; hi != aHolesEnd ; ++ hi )
    ssb.enter_contour( CGAL_SS_i::vertices_begin(*hi), CGAL_SS_i::vertices_end(*hi), Converter ) ;

  return ssb.construct_skeleton();
}

template<class FT, class PointIterator, class K>
boost::shared_ptr< Straight_skeleton_2<K> >
create_partial_exterior_straight_skeleton_2 ( FT const&      aMaxOffset
                                            , PointIterator  aVerticesBegin
                                            , PointIterator  aVerticesEnd
                                            , K const&       k // aka 'SK'
                                            )
{
  CGAL_precondition( aMaxOffset > static_cast<FT>(0) ) ;

  typedef typename std::iterator_traits<PointIterator>::value_type   Point_2;
  typedef typename Kernel_traits<Point_2>::Kernel                    IK;
  typedef typename IK::FT                                            IFT;

  boost::shared_ptr<Straight_skeleton_2<K> > rSkeleton;

  // That's because we might not have FT == IK::FT (e.g. `double` and `Core`)
  // Note that we can also have IK != K (e.g. `Simple_cartesian<Core>` and `EPICK`)
  IFT lOffset = aMaxOffset;

  // @todo This likely should be done in the kernel K rather than the input kernel (i.e. the same
  // converter stuff that is done in `create_partial_exterior_straight_skeleton_2`?).
  boost::optional<IFT> margin = compute_outer_frame_margin(aVerticesBegin,
                                                           aVerticesEnd,
                                                           lOffset);

  if ( margin )
  {
    double lm = CGAL::to_double(*margin);
    Bbox_2 bbox = bbox_2(aVerticesBegin, aVerticesEnd);

    FT fxmin = bbox.xmin() - lm ;
    FT fxmax = bbox.xmax() + lm ;
    FT fymin = bbox.ymin() - lm ;
    FT fymax = bbox.ymax() + lm ;

    Point_2 frame[4] ;

    frame[0] = Point_2(fxmin,fymin) ;
    frame[1] = Point_2(fxmax,fymin) ;
    frame[2] = Point_2(fxmax,fymax) ;
    frame[3] = Point_2(fxmin,fymax) ;

    typedef std::vector<Point_2> Hole ;

    Hole lPoly(aVerticesBegin, aVerticesEnd);
    std::reverse(lPoly.begin(), lPoly.end());

    std::vector<Hole> holes ;
    holes.push_back(lPoly) ;

    rSkeleton = create_partial_interior_straight_skeleton_2(aMaxOffset,frame, frame+4, holes.begin(), holes.end(), k ) ;
  }

  return rSkeleton ;
}

//
// Kernel != Skeleton::kernel. The skeleton is converted to Straight_skeleton_2<Kernel>
//
template<class OutPolygon, class FT, class Skeleton, class K>
std::vector< boost::shared_ptr<OutPolygon> >
create_offset_polygons_2 ( FT const& aOffset, Skeleton const& aSs, K const& , Tag_false )
{
  typedef boost::shared_ptr<OutPolygon> OutPolygonPtr ;
  typedef std::vector<OutPolygonPtr>    OutPolygonPtrVector ;

  typedef Straight_skeleton_2<K> OfSkeleton ;

  typedef Polygon_offset_builder_traits_2<K>                                  OffsetBuilderTraits;
  typedef Polygon_offset_builder_2<OfSkeleton,OffsetBuilderTraits,OutPolygon> OffsetBuilder;

  OutPolygonPtrVector rR ;

  boost::shared_ptr<OfSkeleton> lConvertedSs = convert_straight_skeleton_2<OfSkeleton>(aSs);
  OffsetBuilder ob( *lConvertedSs );
  ob.construct_offset_contours(aOffset, std::back_inserter(rR) ) ;

  return rR ;
}

//
// Kernel == Skeleton::kernel, no convertion
//
template<class OutPolygon, class FT, class Skeleton, class K>
std::vector< boost::shared_ptr<OutPolygon> >
create_offset_polygons_2 ( FT const& aOffset, Skeleton const& aSs, K const& /*k*/, Tag_true )
{
  typedef boost::shared_ptr<OutPolygon> OutPolygonPtr ;
  typedef std::vector<OutPolygonPtr>    OutPolygonPtrVector ;

  typedef Polygon_offset_builder_traits_2<K>                                OffsetBuilderTraits;
  typedef Polygon_offset_builder_2<Skeleton,OffsetBuilderTraits,OutPolygon> OffsetBuilder;

  OutPolygonPtrVector rR ;

  OffsetBuilder ob(aSs);
  ob.construct_offset_contours(aOffset, std::back_inserter(rR) ) ;

  return rR ;
}

// Allow failure due to invalid straight skeletons to go through the users
template<class Skeleton>
Skeleton const& dereference ( boost::shared_ptr<Skeleton> const& ss )
{
  CGAL_precondition(ss.get() != 0);
  return *ss;
}

} // namespace CGAL_SS_i

template<class Polygon, class FT, class Skeleton, class K>
std::vector< boost::shared_ptr<Polygon> >
inline
create_offset_polygons_2(const FT& aOffset,
                         const Skeleton& aSs,
                         const K& k)
{
  typename CGAL_SS_i::Is_same_type<K, typename Skeleton::Traits>::type same_kernel;
  return CGAL_SS_i::create_offset_polygons_2<Polygon>(aOffset, aSs, k, same_kernel);
}

template<class OfK, class C, class FT, class Skeleton>
std::vector< boost::shared_ptr<Polygon_2<OfK, C> > >
inline
create_offset_polygons_2(const FT& aOffset,
                         const Skeleton& aSs)
{
  return create_offset_polygons_2<Polygon_2<OfK, C> >(aOffset, aSs, OfK());
}

template<class OfK, class C, class FT, class Skeleton>
std::vector< boost::shared_ptr<Polygon_with_holes_2<OfK, C> > >
inline
create_offset_polygons_2(const FT& aOffset,
                         const Skeleton& aSs)
{
  return create_offset_polygons_2<Polygon_with_holes_2<OfK, C> >(aOffset, aSs, OfK());
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
/// INTERIOR

template<class FT, class Polygon, class HoleIterator, class OfK, class SsK>
std::vector< boost::shared_ptr<Polygon> >
inline
create_interior_skeleton_and_offset_polygons_2(const FT& aOffset,
                                               const Polygon& aOuterBoundary,
                                               HoleIterator aHolesBegin,
                                               HoleIterator aHolesEnd,
                                               const OfK& ofk,
                                               const SsK& ssk)
{
  return create_offset_polygons_2<Polygon>(
           aOffset,
           CGAL_SS_i::dereference(
             CGAL_SS_i::create_partial_interior_straight_skeleton_2(
               aOffset,
               CGAL_SS_i::vertices_begin(aOuterBoundary),
               CGAL_SS_i::vertices_end  (aOuterBoundary),
               aHolesBegin,
               aHolesEnd,
               ssk)),
           ofk);
}

template<class FT, class Polygon, class HoleIterator, class OfK>
std::vector< boost::shared_ptr<Polygon> >
inline
create_interior_skeleton_and_offset_polygons_2(const FT& aOffset,
                                               const Polygon& aOuterBoundary,
                                               HoleIterator aHolesBegin,
                                               HoleIterator aHolesEnd,
                                               const OfK& ofk)
{
  return create_interior_skeleton_and_offset_polygons_2(aOffset, aOuterBoundary,
                                                        aHolesBegin, aHolesEnd,
                                                        ofk, ofk);
}

template<class FT, class Polygon, class OfK, class SsK>
std::vector< boost::shared_ptr<Polygon> >
inline
create_interior_skeleton_and_offset_polygons_2(const FT& aOffset,
                                               const Polygon& aPoly,
                                               const OfK& ofk,
                                               const SsK& ssk)
{
  std::vector<Polygon> no_holes;
  return create_interior_skeleton_and_offset_polygons_2(aOffset, aPoly,
                                                        no_holes.begin(), no_holes.end(),
                                                        ofk, ssk);
}

template<class FT, class Polygon, class OfK>
std::vector< boost::shared_ptr<Polygon> >
inline
create_interior_skeleton_and_offset_polygons_2(const FT& aOffset,
                                               const Polygon& aPoly,
                                               const OfK& ofk)
{
  std::vector<Polygon> no_holes ;
  return create_interior_skeleton_and_offset_polygons_2(aOffset, aPoly,
                                                        no_holes.begin(), no_holes.end(),
                                                        ofk, ofk);
}

template<class FT, class OfK, class C>
std::vector< boost::shared_ptr<Polygon_2<OfK, C> > >
inline
create_interior_skeleton_and_offset_polygons_2(const FT& aOffset,
                                               const Polygon_2<OfK, C>& aPoly)
{
  return create_interior_skeleton_and_offset_polygons_2(aOffset, aPoly, OfK());
}

template<class FT, class OfK, class C>
std::vector< boost::shared_ptr<Polygon_with_holes_2<OfK, C>> >
inline
create_interior_skeleton_and_offset_polygons_2(const FT& aOffset,
                                               const Polygon_with_holes_2<OfK, C>& aPoly)
{
  return create_interior_skeleton_and_offset_polygons_2(aOffset, aPoly, OfK());
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
/// EXTERIOR

template<class FT, class Polygon, class OfK, class SsK>
std::vector< boost::shared_ptr<Polygon> >
inline
create_exterior_skeleton_and_offset_polygons_2(const FT& aOffset,
                                               const Polygon& aPoly,
                                               const OfK& ofk,
                                               const SsK& ssk)
{
  return create_offset_polygons_2<Polygon>(
           aOffset,
           CGAL_SS_i::dereference(
             CGAL_SS_i::create_partial_exterior_straight_skeleton_2(
               aOffset,
               CGAL_SS_i::vertices_begin(aPoly),
               CGAL_SS_i::vertices_end  (aPoly),
               ssk)),
           ofk);
}

template<class FT, class Polygon, class OfK>
std::vector< boost::shared_ptr<Polygon> >
inline
create_exterior_skeleton_and_offset_polygons_2(const FT& aOffset,
                                               const Polygon& aPoly,
                                               const OfK& ofk)
{
  return create_exterior_skeleton_and_offset_polygons_2(aOffset, aPoly, ofk, ofk);
}

template<class FT, class OfK, class C>
std::vector< boost::shared_ptr<Polygon_2<OfK, C> > >
inline
create_exterior_skeleton_and_offset_polygons_2(const FT& aOffset,
                                               const Polygon_2<OfK, C>& aPoly)
{
  return create_exterior_skeleton_and_offset_polygons_2(aOffset, aPoly, OfK());
}

template<class FT, class OfK, class C>
std::vector<boost::shared_ptr<Polygon_with_holes_2<OfK, C> > >
inline
create_exterior_skeleton_and_offset_polygons_2(const FT& aOffset,
                                               const Polygon_with_holes_2<OfK, C>& aPoly)
{
  return create_exterior_skeleton_and_offset_polygons_2(aOffset, aPoly, OfK());
}

} // end namespace CGAL

#endif
// EOF //
