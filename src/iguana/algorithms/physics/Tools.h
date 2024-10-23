/// @file Tools.h

#include <optional>
#include <Math/Vector3D.h>

namespace iguana::physics::tools {

  /// a value used when some calculation fails
  double const UNDEF{-10000};

  /// @brief calculate the angle between two planes
  ///
  /// The two planes are transverse to @latex{\vec{v}_a\times\vec{v}_b} and @latex{\vec{v}_c\times\vec{v}_d}
  /// @param v_a vector @latex{\vec{v}_a}
  /// @param v_b vector @latex{\vec{v}_b}
  /// @param v_c vector @latex{\vec{v}_c}
  /// @param v_d vector @latex{\vec{v}_d}
  /// @returns the angle between the planes, in radians, if the calculation is successful
  std::optional<double> PlaneAngle(
      ROOT::Math::XYZVector const v_a,
      ROOT::Math::XYZVector const v_b,
      ROOT::Math::XYZVector const v_c,
      ROOT::Math::XYZVector const v_d);

  /// @brief projection of one vector onto another
  /// @param v_a vector @latex{\vec{v}_a}
  /// @param v_b vector @latex{\vec{v}_b}
  /// @returns the vector @latex{\vec{v}_a} projected onto vector @latex{\vec{v}_b}, if the calculation is successful
  std::optional<ROOT::Math::XYZVector> ProjectVector(
      ROOT::Math::XYZVector const v_a,
      ROOT::Math::XYZVector const v_b);

  /// @brief projection of one vector onto the plane transverse to another vector
  /// @param v_a vector @latex{\vec{v}_a}
  /// @param v_b vector @latex{\vec{v}_b}
  /// @returns the vector @latex{\vec{v}_a} projected onto the plane transverse to @latex{\vec{v}_b}, if the calculation is successful
  std::optional<ROOT::Math::XYZVector> RejectVector(
      ROOT::Math::XYZVector const v_a,
      ROOT::Math::XYZVector const v_b);

  /// @brief calculate the angle between two vectors
  /// @param v_a vector @latex{\vec{v}_a}
  /// @param v_b vector @latex{\vec{v}_b}
  /// @returns the angle between @latex{\vec{v}_a} and @latex{\vec{v}_b}, if the calculation is successful
  std::optional<double> VectorAngle(
      ROOT::Math::XYZVector const v_a,
      ROOT::Math::XYZVector const v_b);

}
