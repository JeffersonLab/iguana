#pragma once

#include "iguana/algorithms/Algorithm.h"

namespace iguana::clas12 {

  ///
  /// @brief_algo This is a template algorithm, used as an example showing how to write an algorithm.
  ///
  /// Provide a more detailed description of your algorithm here.
  ///
  /// @begin_doc_algo{clas12::RGAFiducialFilter | Filter}
  /// @input_banks{REC::Particle}
  /// @output_banks{REC::Particle}
  /// @end_doc
  ///
  /// @begin_doc_config{example/RGAFiducialFilter}
  /// @config_param{exampleInt | int | an example `integer` configuration parameter}
  /// @config_param{exampleDouble | double | an example `double` configuration parameter}
  /// @end_doc
  class RGAFiducialFilter : public Algorithm
  {

      DEFINE_IGUANA_ALGORITHM(RGAFiducialFilter, clas12::RGAFiducialFilter)

    public:

      void Start(hipo::banklist& banks) override;
      void Run(hipo::banklist& banks) const override;
      void Stop() override;

      /// @action_function{scalar filter} checks if the PDG `pid` is positive;
      /// this is an example action function, please replace it with your own
      /// @param pid the particle PDG to check
      /// @returns `true` if `pid` is positive
      bool Filter(int const pid) const;

    private:

      /// `hipo::banklist` index for the particle bank (as an example)
      hipo::banklist::size_type b_particle;

      /// Example integer configuration option
      int o_exampleInt;
      /// Example double configuration option
      double o_exampleDouble;
  };

}
