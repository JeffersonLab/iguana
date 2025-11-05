/// @file Tools.h

#include <Math/Vector3D.h>
#include <Math/Vector4D.h>
#include <optional>

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

  /// @brief calculate the rapidity of a particle, relative to an axis
  ///
  /// Given a particle momentum, this method calculates the rapidity
  /// of the boost along an axis which takes an observer to
  /// the frame in which the particle is moving perpendicular to the axis
  /// @param momentum_vec the particle 4-momentum
  /// @param axis_vec the axis 3-vector
  /// @returns the rapidity
  template <typename MOMENTUM_TYPE, typename AXIS_TYPE>
  std::optional<double> ParticleRapidity(
      MOMENTUM_TYPE const& momentum_vec,
      AXIS_TYPE const& axis_vec);

  /// @brief shift angle to the range @latex{(-\pi,+\pi]}
  /// @param ang the angle, in radians
  /// @returns the adjusted angle
  double AdjustAnglePi(double ang);

  /// @brief shift angle to the range @latex{(0,2\pi]}
  /// @param ang the angle, in radians
  /// @returns the adjusted angle
  double AdjustAngleTwoPi(double ang);

}
