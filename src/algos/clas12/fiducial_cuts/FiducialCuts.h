# pragma once

// #include "iguana/Algorithm.h"

namespace iguana::clas12 {

  class FiducialCuts /*: public Algorithm*/ {

    public:
      FiducialCuts() /*: Algorithm()*/ {}
      ~FiducialCuts() {}

      void Start()/* override*/;
      int Run(int a, int b)/* override*/;
      void Stop()/* override*/;
  };
}
