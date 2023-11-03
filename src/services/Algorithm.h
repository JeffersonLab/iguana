# pragma once

namespace iguana {

  class Algorithm {

    public:
      Algorithm();
      virtual void Start() = 0;
      virtual int Run(int a, int b) = 0;
      virtual void Stop() = 0;
      virtual ~Algorithm() {}

  };
}
