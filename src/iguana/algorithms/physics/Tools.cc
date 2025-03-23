#include "Tools.h"

namespace iguana::physics::tools {

  std::optional<double> PlaneAngle(
      ROOT::Math::XYZVector const v_a,
      ROOT::Math::XYZVector const v_b,
      ROOT::Math::XYZVector const v_c,
      ROOT::Math::XYZVector const v_d)
  {
    auto cross_ab = v_a.Cross(v_b); // A x B
    auto cross_cd = v_c.Cross(v_d); // C x D

    auto sgn = cross_ab.Dot(v_d); // (A x B) . D
    if(!(std::abs(sgn) > 0))
      return std::nullopt;
    sgn /= std::abs(sgn); // sign of (A x B) . D

    auto numer = cross_ab.Dot(cross_cd); // (A x B) . (C x D)
    auto denom = cross_ab.R() * cross_cd.R(); // |A x B| * |C x D|
    if(!(std::abs(denom) > 0))
      return std::nullopt;
    return sgn * std::acos(numer / denom);
  }

  std::optional<ROOT::Math::XYZVector> ProjectVector(
      ROOT::Math::XYZVector const v_a,
      ROOT::Math::XYZVector const v_b)
  {
    auto denom = v_b.Dot(v_b);
    if(!(std::abs(denom) > 0))
      return std::nullopt;
    return v_b * ( v_a.Dot(v_b) / denom );
  }

  std::optional<ROOT::Math::XYZVector> RejectVector(
      ROOT::Math::XYZVector const v_a,
      ROOT::Math::XYZVector const v_b)
  {
    auto v_c = ProjectVector(v_a, v_b);
    if(v_c.has_value())
      return v_a - v_c.value();
    return std::nullopt;
  }

  std::optional<double> VectorAngle(
      ROOT::Math::XYZVector const v_a,
      ROOT::Math::XYZVector const v_b)
  {
    double m = v_a.R() * v_b.R();
    if(m > 0)
      return std::acos(v_a.Dot(v_b) / m);
    return std::nullopt;
  }

  template <typename MOMENTUM_TYPE, typename AXIS_TYPE>
  std::optional<double> ParticleRapidity(
      MOMENTUM_TYPE const& momentum_vec,
      AXIS_TYPE const& axis_vec)
  {
    auto norm = axis_vec.R();
    if(std::abs(norm) > 0) {
      auto pz = momentum_vec.Vect().Dot(axis_vec) / norm;
      auto e  = momentum_vec.E();
      return 0.5 * std::log((e + pz) / (e - pz));
    }
    return std::nullopt;
  }
  template std::optional<double> ParticleRapidity(
      ROOT::Math::LorentzVector<ROOT::Math::PxPyPzM4D<double>> const& momentum_vec,
      ROOT::Math::DisplacementVector3D<ROOT::Math::Cartesian3D<double>> const& axis_vec);
  template std::optional<double> ParticleRapidity(
      ROOT::Math::LorentzVector<ROOT::Math::PxPyPzE4D<double>> const& momentum_vec,
      ROOT::Math::DisplacementVector3D<ROOT::Math::Cartesian3D<double>> const& axis_vec);

}

