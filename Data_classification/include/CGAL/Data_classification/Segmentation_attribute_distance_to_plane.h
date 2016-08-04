#ifndef CGAL_DATA_CLASSIFICATION_SEGMENTATION_ATTRIBUTE_DISTANCE_TO_PLANE_H
#define CGAL_DATA_CLASSIFICATION_SEGMENTATION_ATTRIBUTE_DISTANCE_TO_PLANE_H

#include <vector>

#include <CGAL/Point_set_classification.h>

namespace CGAL {

  /*!
    \ingroup PkgDataClassification

    \brief Segmentation attribute based on local non-planarity.

    Characterizing a level of non-planarity can help identify noisy
    parts of the input such as vegetation. This attribute computes the
    distance of a point to a locally fitted plane.
    
    \tparam Kernel The geometric kernel used.

  */
template <typename Kernel, typename RandomAccessIterator, typename PointPMap,
          typename DiagonalizeTraits = CGAL::Default_diagonalize_traits<double,3> >
class Segmentation_attribute_distance_to_plane : public Segmentation_attribute
{
  typedef Data_classification::Local_eigen_analysis<Kernel, RandomAccessIterator,
                                                    PointPMap, DiagonalizeTraits> Local_eigen_analysis;

  std::vector<double> distance_to_plane_attribute;
  
public:
  /// \cond SKIP_IN_MANUAL
  double weight;
  double mean;
  double max;
  /// \endcond
  
  
  /*!
    \brief Constructs the attribute.

    \param on_groups Select if the attribute is computed point-wise of group-wise
  */
  Segmentation_attribute_distance_to_plane (RandomAccessIterator begin,
                                            RandomAccessIterator end,
                                            PointPMap point_pmap,
                                            const Local_eigen_analysis& eigen,
                                            double weight = 1.) : weight (weight)
  {
    for(std::size_t i = 0; i < (std::size_t)(end - begin); i++)
      distance_to_plane_attribute.push_back
        (std::sqrt (CGAL::squared_distance (get(point_pmap, begin[i]), eigen.plane(i))));
    
    this->compute_mean_max (distance_to_plane_attribute, mean, max);
    //    max *= 2;
  }

  virtual double value (std::size_t pt_index)
  {
    return std::max (0., std::min (1., distance_to_plane_attribute[pt_index] / weight));
  }

  virtual std::string id() { return "distance_to_plane"; }
};


}

#endif // CGAL_DATA_CLASSIFICATION_SEGMENTATION_ATTRIBUTE_DISTANCE_TO_PLANE_H
